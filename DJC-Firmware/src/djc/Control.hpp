// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <MAVLink.h>

#include <kf/Function.hpp>
#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/DeviceState.hpp"
#include "djc/InputHandler.hpp"
#include "djc/prelude.hpp"

namespace djc {

namespace internal {

enum class ControlMode : kf::u8 {
    Raw,
    MavLink,
};

struct ControlConfig final : kf::mixin::NonCopyable {
    ControlMode mode;
    kf::math::Milliseconds heartbeat_period, control_send_period, debug_axis_period;
};

}// namespace internal

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control>, kf::mixin::Configurable<internal::ControlConfig>, kf::mixin::Initable<Control, bool> {

    using Mode = internal::ControlMode;
    using Config = internal::ControlConfig;

    using RawMessageCallback = kf::Function<void(kf::memory::Slice<const kf::u8>)>;
    using MavLinkMessageCallback = kf::Function<void(mavlink_message_t *)>;

    struct RawData {
        using Unit = kf::i16;

        static constexpr Unit scale{1000};

        Unit left_x, left_y, right_x, right_y;

        static constexpr Unit fromReal(kf::f32 value) noexcept {
            return static_cast<Unit>(value * scale);
        }

        static constexpr RawData fromControllerValues(const InputHandler::ControllerValues &controller_values) noexcept {
            return RawData{
                .left_x = RawData::fromReal(controller_values.left_x),
                .left_y = RawData::fromReal(controller_values.left_y),
                .right_x = RawData::fromReal(controller_values.right_x),
                .right_y = RawData::fromReal(controller_values.right_y),
            };
        }
    };

    explicit Control(const Config &config, DeviceState &device_state, InputHandler &input_handler, kf::Option<EspNow::Peer> &peer_option) noexcept :
        kf::mixin::Configurable<Config>{config},
        _device_state{device_state}, _input_handler{input_handler}, _peer_option{peer_option} {}

    void onRawMessage(RawMessageCallback &&callback) noexcept { _raw_message_callback = std::move(callback); }

    void onMavlinkMessage(MavLinkMessageCallback &&callback) noexcept { _mavlink_message_callback = std::move(callback); }

private:
    DeviceState &_device_state;
    InputHandler &_input_handler;
    kf::Option<EspNow::Peer> &_peer_option;
    RawMessageCallback _raw_message_callback{};
    MavLinkMessageCallback _mavlink_message_callback{};

    kf::math::Timer _control_send_timer{this->config().control_send_period};
    kf::math::Timer _heartbear_timer{this->config().heartbeat_period};

    void onReceive(kf::memory::Slice<const kf::u8> buffer) noexcept {
        switch (this->config().mode) {
            case Mode::Raw:
                onReceiveRaw(buffer);
                return;

            case Mode::MavLink:
                onReceiveMavLink(buffer);
                return;
        }
    }

    void onReceiveRaw(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (_raw_message_callback) {
            _raw_message_callback(buffer);
        }
    }

    void onReceiveMavLink(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (_mavlink_message_callback) {
            mavlink_message_t message;
            mavlink_status_t status;

            for (auto b: buffer) {
                if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status) != 0) {
                    _mavlink_message_callback(&message);
                }
            }
        }
    }

    void pollRaw(EspNow::Peer &peer, kf::math::Milliseconds now) noexcept {
        if (_control_send_timer.expired(now)) {
            _control_send_timer.start(now);
            peer.writePacket(RawData::fromControllerValues(_input_handler.controllerValues()));
        }
    }

    void pollMavLink(EspNow::Peer &peer, kf::math::Milliseconds now) noexcept {
        if (_control_send_timer.expired(now)) {
            _control_send_timer.start(now);
            sendMavLinkControl(peer);
        }

        if (_heartbear_timer.expired(now)) {
            _heartbear_timer.start(now);
            sendMavLinkHeartbeat(peer);
        }
    }

    void sendMavLinkControl(EspNow::Peer &peer) noexcept {
        const auto raw = RawData::fromControllerValues(_input_handler.controllerValues());

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x: pitch (right Y)
            raw.right_y,
            // y: roll (right X)
            raw.right_x,
            // z: thrust (left Y)
            raw.left_y,
            // r: yaw (left X)
            raw.left_x,
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavLinkMessage(peer, &message);
    }

    void sendMavLinkHeartbeat(EspNow::Peer &peer) noexcept {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127,            // System ID
            MAV_COMP_ID_OSD,// Component ID
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0// Base mode, Custom mode, system status
        );

        sendMavLinkMessage(peer, &message);
    }

    void sendMavLinkMessage(EspNow::Peer &peer, mavlink_message_t *message) noexcept {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, message);
        peer.writeBuffer(kf::memory::Slice<const kf::u8>{buffer, len});
    }

    // impl
    KF_IMPL_INITABLE(Control, bool);
    bool initImpl() noexcept {
        EspNow::instance().onReceiveFromUnknown([this](const EspNow::Mac &, kf::memory::Slice<const kf::u8> buffer) {
            onReceive(buffer);
        });
    }

    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_device_state.menu_navigation_enabled or not _peer_option.hasValue()) { return; }

        // if (_debug_log_timer.expired(now)) {
        //     logger.debug(kf::memory::ArrayString<64>::formatted(
        //                      "L: (%.3f, %.3f) R: (%.3f, %.3f)",
        //                      left_x, left_y, right_x, right_y)
        //                      .view());
        //     _debug_log_timer.start(now);
        // }

        switch (this->config().mode) {
            case Mode::Raw:
                pollRaw(_peer_option.value(), now);
                return;

            case Mode::MavLink:
                pollMavLink(_peer_option.value(), now);
                return;
        }
    }
};

}// namespace djc

/*


    void onMavLinkMessage(mavlink_message_t *message) noexcept {

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                constexpr auto len{sizeof(serial_control.data)};

                serial_control.data[len - 1] = '\0';

                // callback?
                // todo
                return;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                // callback?
                // todo
                return;
            }

            default:
                // Unhandled message type
                return;
        }
*/
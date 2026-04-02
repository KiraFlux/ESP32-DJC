// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <MAVLink.h>

#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Configurable.hpp>
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
    kf::math::Milliseconds heartbeat_period, poll_period;
};

}// namespace internal

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control>, kf::mixin::Configurable<internal::ControlConfig> {

    using Mode = internal::ControlMode;
    using Config = internal::ControlConfig;

    struct RawData {
        using Unit = kf::i16;

        static constexpr Unit scale{1000};

        Unit left_x, left_y, right_x, right_y;

        constexpr static Unit fromReal(kf::f32 value) noexcept {
            return static_cast<Unit>(value * scale);
        }
    };

    explicit Control(DeviceState &device_state, InputHandler &input_handler, kf::Option<EspNow::Peer> &peer_option) noexcept :
        _device_state{device_state}, _input_handler{input_handler}, _peer_option{peer_option} {}

private:
    DeviceState &_device_state;
    InputHandler &_input_handler;
    kf::Option<EspNow::Peer> &_peer_option;
    kf::math::Timer _poll_timer{this->config().poll_period};

    void sendRawControl(EspNow::Peer &peer) noexcept {
        const auto &control_values = _input_handler.controllerValues();

        peer.writePacket(RawData{
            .left_x = RawData::fromReal(control_values.left_x),
            .left_y = RawData::fromReal(control_values.left_y),
            .right_x = RawData::fromReal(control_values.right_x),
            .right_y = RawData::fromReal(control_values.right_y),
        });
    }

    void sendMavLinkControl(EspNow::Peer &peer) noexcept {

    }

    void sendMavLinkMessage(EspNow::Peer &peer) noexcept {
        
    }

    // impl
    KF_IMPL_TIMED_POLLABLE(Control);

    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_device_state.menu_navigation_enabled or not _peer_option.hasValue()) { return; }

        if (_poll_timer.expired(now)) {
            _poll_timer.start(now);

            switch (this->config().mode) {
                case Mode::Raw:
                    sendRawControl(_peer_option.value());
                    return;

                case Mode::MavLink:
                    sendMavLinkControl(_peer_option.value());
                    return;
            }
        }
    }
};

}// namespace djc

/*
    void onMavLinkMessage(mavlink_message_t *message) {
        logger.debug(kf::memory::ArrayString<32>::formatted("MAVLink message ID: %d", message->msgid).view());

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(log_buffer.data(), serial_control.data, serial_control.count);
                log_buffer[kf::min(kf::usize(serial_control.count), log_buffer.size() - 1)] = '\0';
                log_display.value(log_buffer.view());
                break;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) log_buffer.format(
                    "Accel: %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f);
                log_display.value(log_buffer.view());
                break;
            }

            default:
                // Unhandled message type
                break;
        }
    }

    static void sendHeartBeat() {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127,            // System ID
            MAV_COMP_ID_OSD,// Component ID
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0// Base mode, Custom mode, system status
        );

        sendMavlinkToEspnow(message);
    }

    void sendManualControl(kf::math::Milliseconds now) {
        constexpr auto scale = 1000;

        const auto &controller_values = Device::instance().controllerValues();
        const auto left_x = controller_values.left_x * scale;
        const auto left_y = controller_values.left_y * scale;
        const auto right_x = controller_values.right_x * scale;
        const auto right_y = controller_values.right_y * scale;

        if (debug_timer.expired(now)) {
            logger.debug(kf::memory::ArrayString<64>::formatted(
                             "L: (%.3f, %.3f) R: (%.3f, %.3f)",
                             left_x, left_y, right_x, right_y)
                             .view());
            debug_timer.start(now);
        }

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x: pitch (right Y)
            static_cast<kf::i16>(right_y),
            // y: roll (right X)
            static_cast<kf::i16>(right_x),
            // z: thrust (left Y)
            static_cast<kf::i16>(left_y),
            // r: yaw (left X)
            static_cast<kf::i16>(left_x),
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        auto &peer_opt = Device::instance().espnowPeer();
        if (peer_opt.hasValue()) {
            (void) peer_opt.value().writeBuffer(kf::memory::Slice<const kf::u8>{buffer, len});
        }
    }
*/
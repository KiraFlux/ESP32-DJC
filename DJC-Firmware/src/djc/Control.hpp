// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <MAVLink.h>

#include <kf/Function.hpp>
#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>
#include <kf/network/EspNow.hpp>

#include "djc/DeviceState.hpp"
#include "djc/InputHandler.hpp"

namespace djc {

namespace internal {

enum class ControlMode : kf::u8 {
    Raw,
    MavLink,
};

struct ControlConfig final : kf::mixin::NonCopyable {
    kf::math::Milliseconds heartbeat_period;
    kf::math::Milliseconds poll_period;

    ControlMode mode;

    static constexpr ControlConfig defaults() noexcept {
        return ControlConfig{
            .heartbeat_period = 2000,                                     // ms
            .poll_period = static_cast<kf::math::Milliseconds>(1000 / 50),// 50 Hz
            .mode = ControlMode::Raw,
        };
    }
};

}// namespace internal

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control>, kf::mixin::Configurable<internal::ControlConfig>, kf::mixin::Initable<Control, bool> {

    using EspNow = kf::network::EspNow;

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

    explicit Control(const Config &config, DeviceState &device_state, InputHandler &input_handler) noexcept :
        kf::mixin::Configurable<Config>{config},
        _device_state{device_state}, _input_handler{input_handler} {}

    void activePeer(EspNow::Mac &mac) noexcept {
        if (_active_peer.hasValue()) {
            auto &peer = _active_peer.value();
            if (peer.mac() == mac) { return; }// same -> leave

            // delete only if old peer yet exists
            if (peer.exist()) {
                const auto result = peer.del();
                logger.error(
                    kf::memory::ArrayString<64>::formatted(
                        "Failed to delete peer [%s] : %s",
                        EspNow::stringFromMac(peer.mac()).data(),
                        EspNow::stringFromError(result.error()))
                        .view());
            }
        }

        auto result = EspNow::Peer::add(mac);
        if (result.isError()) {
            logger.error(
                kf::memory::ArrayString<64>::formatted(
                    "Failed to add peer [%s] :%s",
                    EspNow::stringFromMac(mac).data(),
                    EspNow::stringFromError(result.error()))
                    .view());
            return;
        }

        _active_peer.value(std::move(result.value()));
    }

    kf::Option<EspNow::Mac> activePeer() noexcept {
        if (_active_peer.hasValue()) {
            return {_active_peer.value().mac()};
        } else {
            return {};
        }
    }

    void onReceiveFromUnknown(EspNow::ReceiveFromUnknownHandler &&callback) noexcept { EspNow::instance().onReceiveFromUnknown(std::move(callback)); }

    void onRawMessage(RawMessageCallback &&callback) noexcept { _raw_message_callback = std::move(callback); }

    void onMavlinkMessage(MavLinkMessageCallback &&callback) noexcept { _mavlink_message_callback = std::move(callback); }

private:
    static constexpr auto logger{kf::Logger::create("Control")};

    DeviceState &_device_state;
    InputHandler &_input_handler;
    kf::Option<EspNow::Peer> _active_peer{};
    RawMessageCallback _raw_message_callback{};
    MavLinkMessageCallback _mavlink_message_callback{};

    kf::math::Timer _poll_timer{this->config().poll_period};
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
        if (_raw_message_callback) { _raw_message_callback(buffer); }
    }

    void onReceiveMavLink(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (not _mavlink_message_callback) { return; }

        mavlink_message_t message;
        mavlink_status_t status;

        for (auto b: buffer) {
            if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status) != 0) {
                _mavlink_message_callback(&message);
            }
        }
    }

    void pollRaw(EspNow::Peer &peer, kf::math::Milliseconds, const RawData &raw) noexcept {
        (void) peer.writePacket(raw);
    }

    void pollMavLink(EspNow::Peer &peer, kf::math::Milliseconds now, const RawData &raw) noexcept {
        sendMavLinkControl(peer, raw);

        if (_heartbear_timer.expired(now)) {
            _heartbear_timer.start(now);
            sendMavLinkHeartbeat(peer);
        }
    }

    void sendMavLinkControl(EspNow::Peer &peer, const RawData &raw) noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            raw.right_y,// x: pitch (right Y)
            raw.right_x,// y: roll (right X)
            raw.left_y, // z: thrust (left Y)
            raw.left_x, // r: yaw (left X)
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavLinkMessage(peer, &message);
    }

    void sendMavLinkHeartbeat(EspNow::Peer &peer) noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_heartbeat_pack(
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
        (void) peer.writeBuffer(kf::memory::Slice<const kf::u8>{buffer, len});
    }

    // impl

    KF_IMPL_INITABLE(Control, bool);
    bool initImpl() noexcept {
        logger.info("init");

        const auto result = EspNow::instance().init();
        if (result.isError()) {
            logger.error("Failed to initialize ESP-NOW: %s");
            return false;
        }

        logger.debug("init: ok");
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_device_state.menu_navigation_enabled or not _active_peer.hasValue()) { return; }

        // if (_debug_log_timer.expired(now)) {
        //     logger.debug(kf::memory::ArrayString<64>::formatted(
        //                      "L: (%.3f, %.3f) R: (%.3f, %.3f)",
        //                      left_x, left_y, right_x, right_y)
        //                      .view());
        //     _debug_log_timer.start(now);
        // }

        if (_poll_timer.expired(now)) {
            _poll_timer.start(now);

            const auto raw = RawData::fromControllerValues(_input_handler.controllerValues());

            switch (this->config().mode) {
                case Mode::Raw:
                    pollRaw(_active_peer.value(), now, raw);
                    return;

                case Mode::MavLink:
                    pollMavLink(_active_peer.value(), now, raw);
                    return;
            }
        }
    }
};

}// namespace djc
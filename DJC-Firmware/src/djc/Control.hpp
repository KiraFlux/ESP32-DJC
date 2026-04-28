// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

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

#include "djc/transport/TransportLink.hpp"

namespace djc {

namespace internal {

enum class ControlMode : kf::u8 {
    Raw,
    MavLink,
};

struct ControlConfig final : kf::mixin::NonCopyable {
    kf::math::Milliseconds heartbeat_period;
    kf::math::Milliseconds poll_period;
    ControlMode init_mode;

    static constexpr ControlConfig defaults() noexcept {
        return ControlConfig{
            .heartbeat_period = 2000,                                     // ms
            .poll_period = static_cast<kf::math::Milliseconds>(1000 / 50),// 50 Hz
            .init_mode = ControlMode::MavLink,
        };
    }
};

}// namespace internal

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control>, kf::mixin::Configurable<internal::ControlConfig>, kf::mixin::Initable<Control, void> {
    using Config = internal::ControlConfig;
    using Mode = internal::ControlMode;

    using RawMessageCallback = kf::Function<void(kf::memory::Slice<const kf::u8>)>;
    using MavLinkMessageCallback = kf::Function<void(const mavlink_message_t *)>;

    struct Input {
        using Unit = kf::i16;

        static constexpr Unit scale{1000};

        Unit left_x, left_y, right_x, right_y;

        static constexpr Unit fromNormalized(kf::f32 value) noexcept { return static_cast<Unit>(value * scale); }
    };

    static constexpr kf::u8 mavlink_system_id{127}, mavlink_target_id{1};

    [[nodiscard]] static constexpr kf::memory::StringView stringFromMode(Mode mode) noexcept { return (mode == Mode::Raw) ? "Raw" : "MavLink"; }

    explicit Control(const Config &config, transport::TransportLink &transport_link) noexcept :
        kf::mixin::Configurable<Config>{config}, _transport_link{transport_link} {}

    // properties

    void onRawMessage(RawMessageCallback &&callback) noexcept { _raw_message_callback = std::move(callback); }

    void onMavlinkMessage(MavLinkMessageCallback &&callback) noexcept { _mavlink_message_callback = std::move(callback); }

    [[nodiscard]] Mode mode() const noexcept { return _mode; }

    void mode(Mode new_mode) noexcept { _mode = new_mode; }

    [[nodiscard]] const Input &input() const noexcept { return _input; }

    void input(const Input &new_input) noexcept { _input = new_input; }

    [[nodiscard]] bool enabled() const noexcept { return _enabled; }

    void enabled(bool is_enabled) noexcept { _enabled = is_enabled; }

    // methods

    void sendMavLinkMessage(const mavlink_message_t *message) noexcept {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, message);

        (void) _transport_link.send({buffer, len});
    }

    void sendRawMessage(kf::memory::Slice<const kf::u8> buffer) noexcept {
        (void) _transport_link.send(buffer);
    }

private:
    using LogString = kf::memory::ArrayString<64>;

    static constexpr auto logger{kf::Logger::create("Control")};

    RawMessageCallback _raw_message_callback{};
    MavLinkMessageCallback _mavlink_message_callback{};

    transport::TransportLink &_transport_link;

    kf::math::Timer _poll_timer{this->config().poll_period};
    kf::math::Timer _heartbear_timer{this->config().heartbeat_period};

    Input _input{};
    Mode _mode{this->config().init_mode};
    bool _enabled{false};

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

    void pollRaw(kf::math::Milliseconds) noexcept {
        (void) _transport_link.send({reinterpret_cast<const kf::u8 *>(&_input), sizeof(_input)});
    }

    void pollMavLink(kf::math::Milliseconds now) noexcept {
        sendMavLinkControl();

        if (_heartbear_timer.expired(now)) {
            _heartbear_timer.start(now);
            sendMavLinkHeartbeat();
        }
    }

    void sendMavLinkControl() noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_manual_control_pack(
            mavlink_system_id, MAV_COMP_ID_PARACHUTE, &message, mavlink_target_id,
            _input.right_y,// x: pitch (right Y)
            _input.right_x,// y: roll (right X)
            _input.left_y, // z: thrust (left Y)
            _input.left_x, // r: yaw (left X)
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavLinkMessage(&message);
    }

    void sendMavLinkHeartbeat() noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_heartbeat_pack(
            mavlink_system_id,// System ID
            MAV_COMP_ID_OSD,  // Component ID
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0// Base mode, Custom mode, system status
        );

        sendMavLinkMessage(&message);
    }

    // impl

    KF_IMPL_INITABLE(Control, void);
    void initImpl() noexcept {
        logger.info("init");

        _mode = this->config().init_mode;
        logger.debug(stringFromMode(_mode));

        const auto now = millis();
        _poll_timer.start(now);
        _heartbear_timer.start(now);

        _transport_link.onReceive([this](const transport::PeerAddress &, kf::memory::Slice<const kf::u8> buffer) {
            switch (_mode) {
                case Mode::Raw:
                    onReceiveRaw(buffer);
                    return;

                case Mode::MavLink:
                    onReceiveMavLink(buffer);
                    return;
            }
        });
    }

    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not _enabled) { return; }
        if (not _transport_link.connected()) { return; }

        if (_poll_timer.expired(now)) {
            _poll_timer.start(now);

            switch (_mode) {
                case Mode::Raw:
                    pollRaw(now);
                    return;

                case Mode::MavLink:
                    pollMavLink(now);
                    return;
            }
        }
    }
};

}// namespace djc
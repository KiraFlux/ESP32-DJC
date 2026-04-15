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

#include "djc/prelude.hpp"

namespace djc {

namespace internal {

enum class ControlMode : kf::u8 {
    Raw,
    MavLink,
};

struct ControlConfig final : kf::mixin::NonCopyable {
    kf::math::Milliseconds heartbeat_period;
    kf::math::Milliseconds poll_period;
    kf::math::Milliseconds receive_timeout;
    ControlMode init_mode;

    static constexpr ControlConfig defaults() noexcept {
        return ControlConfig{
            .heartbeat_period = 2000,                                     // ms
            .poll_period = static_cast<kf::math::Milliseconds>(1000 / 50),// 50 Hz
            .receive_timeout = 30'000,                                    // ms
            .init_mode = ControlMode::MavLink,
        };
    }
};

}// namespace internal

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control>, kf::mixin::Configurable<internal::ControlConfig>, kf::mixin::Initable<Control, bool> {
    using Config = internal::ControlConfig;
    using Mode = internal::ControlMode;

    using LogString = kf::memory::ArrayString<64>;

    using RawMessageCallback = kf::Function<void(kf::memory::Slice<const kf::u8>)>;
    using MavLinkMessageCallback = kf::Function<void(mavlink_message_t *)>;
    using ReceiveFromUnknownCallback = EspNow::ReceiveFromUnknownHandler;

    struct Input {
        using Unit = kf::i16;

        static constexpr Unit scale{1000};

        Unit left_x, left_y, right_x, right_y;

        static constexpr Unit fromReal(kf::f32 value) noexcept { return static_cast<Unit>(value * scale); }
    };

    explicit Control(const Config &config) noexcept : kf::mixin::Configurable<Config>{config} {}

    // properties

    void onReceiveFromUnknown(ReceiveFromUnknownCallback &&callback) noexcept { EspNow::instance().onReceiveFromUnknown(std::move(callback)); }

    void onRawMessage(RawMessageCallback &&callback) noexcept { _raw_message_callback = std::move(callback); }

    void onMavlinkMessage(MavLinkMessageCallback &&callback) noexcept { _mavlink_message_callback = std::move(callback); }

    [[nodiscard]] Mode mode() const noexcept { return _mode; }

    [[nodiscard]] static constexpr kf::memory::StringView stringFromMode(Mode mode) noexcept { return (mode == Mode::Raw) ? "Raw" : "MavLink"; }

    void mode(Mode new_mode) noexcept { _mode = new_mode; }

    [[nodiscard]] const Input &input() const noexcept { return _input; }

    void input(const Input &new_input) noexcept { _input = new_input; }

    [[nodiscard]] bool enabled() const noexcept { return _enabled; }

    void enabled(bool is_enabled) noexcept { _enabled = is_enabled; }

    [[nodiscard]] bool connected() const noexcept { return _active_peer.hasValue(); }

    kf::Option<EspNow::Mac> activeMac() noexcept {
        if (connected()) {
            return {_active_peer.value().mac()};
        } else {
            return {};
        }
    }

    void connect(const EspNow::Mac &mac) noexcept {
        if (connected()) {
            if (_active_peer.value().mac() == mac) {
                logger.debug("Already connected to active peer");
                return;
            }

            disconnect();
        }

        _active_peer = addPeer(mac);
        if (not connected()) { return; }

        const auto receive_setup_result = _active_peer.value().onReceive([this](kf::memory::Slice<const kf::u8> buffer) { onReceive(buffer); });
        if (receive_setup_result.isError()) {
            logger.error("Receive callback attachment failed");
            return;
        }

        _got_packet = true;
        logger.info("Connected: OK");
    }

    void disconnect() noexcept {
        if (not connected()) {
            logger.error("Disconnect failed: No active peer");
            return;
        }

        auto &peer = _active_peer.value();
        if (not peer.exist()) {
            logger.error("Disconnect failed: Peer not exit");
            return;
        }

        delPeer(peer);

        _active_peer = {};
        logger.info("Disconnected: OK");
    }

    void sendMavLinkMessage(mavlink_message_t *message) noexcept {
        if (connected()) {
            sendMavLinkMessage(_active_peer.value(), message);
        }
    }

    void sendRawMessage(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (connected()) {
            (void) _active_peer.value().writeBuffer(buffer);
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("Control")};

    RawMessageCallback _raw_message_callback{};
    MavLinkMessageCallback _mavlink_message_callback{};

    kf::Option<EspNow::Peer> _active_peer{};
    kf::Option<EspNow::Peer> _broadcast_peer{};

    kf::math::Timer _poll_timer{this->config().poll_period};
    kf::math::Timer _heartbear_timer{this->config().heartbeat_period};
    kf::math::Timer _receice_disconnect_timer{this->config().receive_timeout};

    Input _input{};
    Mode _mode{this->config().init_mode};
    bool _enabled{false};
    volatile bool _got_packet{false};

    static kf::Option<EspNow::Peer> addPeer(const EspNow::Mac &mac) noexcept {
        auto peer_result = EspNow::Peer::add(mac);
        if (peer_result.isError()) {
            logger.error(
                LogString::formatted(
                    "Failed to add peer [%s] :%s",
                    EspNow::stringFromMac(mac).data(),
                    EspNow::stringFromError(peer_result.error()))
                    .view());
            return {};
        }

        logger.info(LogString::formatted("Peer '%s' added", EspNow::stringFromMac(mac).data()).view());
        return {std::move(peer_result.value())};
    }

    static void delPeer(EspNow::Peer &peer) noexcept {
        const auto result = peer.del();
        if (result.isError()) {
            logger.error(
                LogString::formatted(
                    "Failed to delete peer [%s] : %s",
                    EspNow::stringFromMac(peer.mac()).data(),
                    EspNow::stringFromError(result.error()))
                    .view());
            return;
        }
    }

    void onReceive(kf::memory::Slice<const kf::u8> buffer) noexcept {
        _got_packet = true;
        switch (_mode) {
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

    void pollRaw(EspNow::Peer &peer, kf::math::Milliseconds) noexcept {
        (void) peer.writePacket(_input);
    }

    void pollMavLink(EspNow::Peer &peer, kf::math::Milliseconds now) noexcept {
        sendMavLinkControl(peer);

        if (_heartbear_timer.expired(now)) {
            _heartbear_timer.start(now);
            sendMavLinkHeartbeat(peer);
        }
    }

    void sendMavLinkControl(EspNow::Peer &peer) noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            _input.right_y,// x: pitch (right Y)
            _input.right_x,// y: roll (right X)
            _input.left_y, // z: thrust (left Y)
            _input.left_x, // r: yaw (left X)
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
            logger.error(LogString::formatted("Failed to initialize ESP-NOW: %s", EspNow::stringFromError(result.error())));
            return false;
        }

        _broadcast_peer = addPeer(EspNow::Mac{0xff, 0xff, 0xff, 0xff, 0xff, 0xff});

        _mode = this->config().init_mode;

        const auto now = millis();
        _poll_timer.start(now);
        _heartbear_timer.start(now);

        logger.debug(stringFromMode(_mode));
        logger.debug("init: ok");
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not connected()) { return; }

        if (_got_packet) {
            _got_packet = false;
            _receice_disconnect_timer.start(now);
        }

        if (_receice_disconnect_timer.expired(now)) {
            logger.info("Timeout");
            disconnect();
        }

        if (not _enabled) { return; }

        if (_poll_timer.expired(now)) {
            _poll_timer.start(now);

            switch (_mode) {
                case Mode::Raw:
                    pollRaw(_active_peer.value(), now);
                    return;

                case Mode::MavLink:
                    pollMavLink(_active_peer.value(), now);
                    return;
            }
        }
    }
};

}// namespace djc
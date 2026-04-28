// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

/// @brief Connection manager for a single transport.
/// @note Separates connection lifecycle and inactivity timeout from higher‑level logic.
///       Keeps the transport abstract: the rest of the firmware only talks to `TransportLink`, never to a concrete transport.
struct TransportLink final : kf::mixin::TimedPollable<TransportLink> {

    explicit constexpr TransportLink(kf::math::Milliseconds disconnect_timeout) noexcept :
        _disconnect_timer{disconnect_timeout} {}

    /// @brief Set the active transport, disconnecting any previous connection first.
    /// @note Ensures that changing the transport does not leave a stale connection open,
    ///       which would silently keep receiving data on the old transport.
    void transport(Transport &new_transport) noexcept {
        if (nullptr != _transport and _transport->connected()) {
            _transport->disconnect();
        }

        _transport = &new_transport;
    }

    /// @brief Forward a data buffer to the underlying transport.
    /// @return true if the transport reported success, false on error or if no transport is set.
    [[nodiscard]] bool send(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (nullptr == _transport) {
            logger.error("send failed: no transport set");
            return false;
        }

        return _transport->send(buffer);
    }

    /// @brief Register a callback for incoming data.
    /// @note The callback is invoked for every received packet.
    ///       This method overwrites the transport‑level receive handler so that each incoming packet also resets the inactivity timer.
    void onReceive(Transport::ReceiveCallback &&callback) noexcept {
        if (nullptr == _transport) {
            logger.error("onReceive failed: no transport set");
            return;
        }

        _receive_callback = std::move(callback);

        _transport->onReceive([this](const PeerAddress &address, kf::memory::Slice<const kf::u8> buffer) {
            if (_receive_callback) { _receive_callback(address, buffer); }
            _disconnect_timer_reset_required = true;
        });
    }

    /// @brief Register a callback for incoming data from other peers (non-primary)
    void onReceiveForeign(Transport::ReceiveCallback &&callback) noexcept {
        if (nullptr == _transport) {
            logger.error("onReceiveForeign failed: no transport set");
            return;
        }

        _transport->onReceiveForeign(std::move(callback));
    }

    /// @brief Check whether the transport is currently connected.
    [[nodiscard]] bool connected() const noexcept { return nullptr != _transport and _transport->connected(); }

    /// @brief Return the address of the active peer, if any.
    /// @return Reference to an empty option when no transport is set.
    [[nodiscard]] const kf::Option<PeerAddress> &activePeerAddress() const noexcept {
        return (nullptr == _transport) ? null_option : _transport->activePeerAddress();
    }

    /// @brief Initiate a connection to a peer.
    [[nodiscard]] bool connect(const PeerAddress &address) noexcept {
        if (nullptr == _transport) {
            logger.error("connect failed: no transport set");
            return false;
        }

        _disconnect_timer_reset_required = true;

        return _transport->connect(address);
    }

    /// @brief Disconnect from the current peer.
    void disconnect() noexcept {
        if (nullptr == _transport) {
            logger.error("disconnect failed: no transport set");
            return;
        }

        _transport->disconnect();
    }

private:
    /// @brief Empty address returned when no transport is active.
    static constexpr kf::Option<PeerAddress> null_option{};

    static constexpr auto logger{kf::Logger::create("TransportLink")};

    Transport *_transport{nullptr};                       ///< Currently active transport (may be nullptr).
    Transport::ReceiveCallback _receive_callback{};       ///< User‑supplied callback for incoming data.
    kf::math::Timer _disconnect_timer;                    ///< Inactivity timer.
    volatile bool _disconnect_timer_reset_required{false};///< Flag: reset timer on next poll.

    // impl
    using This = TransportLink;

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not connected()) { return; }

        if (_disconnect_timer_reset_required) {
            _disconnect_timer_reset_required = false;
            _disconnect_timer.start(now);
        }

        if (_disconnect_timer.expired(now)) {
            disconnect();
            logger.info("Disconnect by timeout");
        }
    }
};

}// namespace djc::transport
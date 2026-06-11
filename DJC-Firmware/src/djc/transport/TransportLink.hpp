// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::internal {

/// @brief Configuration parameters for the Transport Link.
struct TransportLinkConfig final : kf::mixin::NonCopyable {
    kf::math::Timer::Config disconnect_timer;

    [[nodiscard]] static constexpr TransportLinkConfig defaults() noexcept {
        return TransportLinkConfig{
            .disconnect_timer = {
                .period = 15'000,
            },
        };
    }
};

}// namespace djc::internal

namespace djc::transport {

/// @brief Connection manager for a single transport.
/// @note Separates connection lifecycle and inactivity timeout from higher‑level logic.
///       Keeps the transport abstract: the rest of the firmware only talks to `TransportLink`, never to a concrete transport.
struct TransportLink final :

    kf::mixin::NonCopyable,
    kf::mixin::Configurable<internal::TransportLinkConfig>,
    kf::mixin::TimedPollable<TransportLink>

{
    using Config = internal::TransportLinkConfig;

    using Configurable<Config>::Configurable;

    /// @brief Set the active transport, disconnecting any previous connection first.
    /// @note Ensures that changing the transport does not leave a stale connection open,
    ///       which would silently keep receiving data on the old transport.
    void transport(Transport &new_transport) noexcept {
        if (_transport.isSome() and _transport.unwrap().connected()) {
            _transport.unwrap().disconnect();
        }

        _transport = kf::someRef(new_transport);
    }

    /// @brief Forward a data buffer to the underlying transport.
    /// @return true if the transport reported success, false on error or if no transport is set.
    [[nodiscard]] bool send(kf::Slice<const kf::u8> buffer) noexcept {
        if (_transport.isNone()) {
            logger.error("send failed: no transport set");
            return false;
        }

        return _transport.unwrap().send(buffer);
    }

    /// @brief Register a callback for incoming data.
    /// @note The callback is invoked for every received packet.
    ///       This method overwrites the transport‑level receive handler so that each incoming packet also resets the inactivity timer.
    void onReceive(Transport::ReceiveCallback &&callback) noexcept {
        if (_transport.isNone()) {
            logger.error("onReceive failed: no transport set");
            return;
        }

        _receive_callback = kf::some(std::move(callback));

        _transport.unwrap().onReceive([this](const PeerAddress &address, kf::Slice<const kf::u8> buffer) {
            if (_receive_callback.isSome()) {
                _receive_callback.unwrap()(address, buffer);
            }
            _disconnect_timer_reset_required = true;
        });
    }

    /// @brief Register a callback for incoming data from other peers (non-primary)
    void onReceiveForeign(Transport::ReceiveCallback &&callback) noexcept {
        if (_transport.isNone()) {
            logger.error("onReceiveForeign failed: no transport set");
            return;
        }

        _transport.unwrap().onReceiveForeign(std::move(callback));
    }

    /// @brief Check whether the transport is currently connected.
    [[nodiscard]] bool connected() const noexcept {
        return _transport.isSome() and _transport.unwrap().connected();
    }

    /// @brief Return the address of the active peer, if any.
    /// @return Reference to an empty option when no transport is set.
    [[nodiscard]] kf::Option<const PeerAddress &> activePeerAddress() const noexcept {
        return _transport.isNone() ? kf::none : _transport.unwrap().activePeerAddress();
    }

    /// @brief Initiate a connection to a peer.
    [[nodiscard]] bool connect(const PeerAddress &address) noexcept {
        if (_transport.isNone()) {
            logger.error("connect failed: no transport set");
            return false;
        }

        _disconnect_timer_reset_required = true;

        return _transport.unwrap().connect(address);
    }

    /// @brief Disconnect from the current peer.
    void disconnect() noexcept {
        if (_transport.isNone()) {
            logger.error("disconnect failed: no transport set");
            return;
        }

        _transport.unwrap().disconnect();
    }

private:
    static constexpr auto logger{kf::Logger::create("TransportLink")};

    kf::Option<Transport &> _transport{kf::none};                      ///< Currently active transport (optional)
    kf::Option<Transport::ReceiveCallback> _receive_callback{kf::none};///< User‑supplied callback for incoming data.
    kf::math::Timer _disconnect_timer{this->config().disconnect_timer};///< Inactivity timer.
    volatile bool _disconnect_timer_reset_required{false};             ///< Flag: reset timer on next poll.

    KF_IMPL_TIMED_POLLABLE(TransportLink);
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
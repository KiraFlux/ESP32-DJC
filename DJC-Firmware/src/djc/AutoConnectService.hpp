// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "transport/PeerAddress.hpp"
#include "transport/TransportLink.hpp"

namespace djc {

namespace internal {

struct AutoConnectServiceConfig final : kf::mixin::NonCopyable {

    kf::math::Milliseconds cooldown;///< Delay before the service reacts to a new target.
    bool enabled;                   ///< Whether the service is active.

    [[nodiscard]] static constexpr AutoConnectServiceConfig defaults() noexcept {
        return AutoConnectServiceConfig{
            .cooldown = 10'000,
            .enabled = true,
        };
    }
};

}// namespace internal

/// @brief Service that automatically connects to a trusted peer after a configurable delay
/// @note
/// Receives a `Target` (address + detection timestamp) from outside.
/// When the timeout expires, the service invokes its callback with the peer address.
/// After the callback, the target is cleared and the service waits for a new one.
struct AutoConnectService final :

    kf::mixin::NonCopyable,
    kf::mixin::Callbacked<const transport::PeerAddress &>,
    kf::mixin::Configurable<internal::AutoConnectServiceConfig>,
    kf::mixin::TimedPollable<AutoConnectService>

{
    /// @brief Configuration for the AutoConnectService
    using Config = internal::AutoConnectServiceConfig;

    /// @brief Information about a detected peer supplied to the service
    struct Target final {
        transport::PeerAddress address;  ///< Address of the peer.
        kf::math::Milliseconds last_seen;///< Timestamp when the peer was last seen (used for cooldown).
    };

    explicit constexpr AutoConnectService(const Config &config, const transport::TransportLink &transport_link) noexcept :
        kf::mixin::Configurable<Config>{config}, _transport_link{transport_link} {}

    [[nodiscard]] const kf::Option<Target> &target() const noexcept { return _target; }

    /// @brief Assign a new target for automatic connection
    /// @note
    /// The target is ignored if the transport is already connected.
    /// This prevents interrupting an active connection.
    void target(const Target &new_target) noexcept {
        if (not _transport_link.connected()) {
            _target.value(new_target);
        }
    }

private:
    const transport::TransportLink &_transport_link;
    kf::Option<Target> _target{};

    KF_IMPL_TIMED_POLLABLE(AutoConnectService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not this->config().enabled) { return; }
        if (not _target.hasValue()) { return; }

        if (now > _target.value().last_seen + this->config().cooldown) {
            this->invoke(_target.value().address);
            _target = {};
        }
    }
};

}// namespace djc
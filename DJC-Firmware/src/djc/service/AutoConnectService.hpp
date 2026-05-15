// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/service/Service.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::service {

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
/// Receives a target from outside.
/// When the timeout expires, the service invokes its callback with the peer address.
/// After the callback, the target is cleared and the service waits for a new one.
struct AutoConnectService final :

    Service<AutoConnectService>,
    kf::mixin::Configurable<internal::AutoConnectServiceConfig>,
    kf::mixin::Callbacked<const transport::PeerAddress &>

{
    /// @brief Configuration for the AutoConnectService
    using Config = internal::AutoConnectServiceConfig;

    explicit AutoConnectService(const Config &config, const transport::TransportLink &transport_link) noexcept :
        kf::mixin::Configurable<Config>{config}, _transport_link{transport_link} {
        _cooldown_timer.start(0);
    }

    [[nodiscard]] const kf::Option<transport::PeerAddress> &target() const noexcept { return _target; }

    /// @brief Assign a new target for automatic connection
    /// @note
    /// The target is ignored if the transport is already connected.
    /// This prevents interrupting an active connection.
    void target(const transport::PeerAddress &new_target) noexcept {
        if (not _transport_link.connected()) {
            _target.value(new_target);
        }
    }

private:
    const transport::TransportLink &_transport_link;
    kf::Option<transport::PeerAddress> _target{};
    kf::math::Timer _cooldown_timer{this->config().cooldown};

    KF_IMPL_TIMED_POLLABLE(AutoConnectService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not this->config().enabled) { return; }
        if (not _target.hasValue()) { return; }

        if (not _cooldown_timer.expired(now)) { return; }

        this->invoke(_target.value());
        _target = {};

        _cooldown_timer.start(now);
    }
};

}// namespace djc::service
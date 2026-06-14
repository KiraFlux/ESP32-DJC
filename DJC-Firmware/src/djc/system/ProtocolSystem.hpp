// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/mixin/Configurable.hpp>

#include "djc/Config.hpp"
#include "djc/MavlinkTelemetryRegistry.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System managing communication protocols and telemetry registry.
/// @note Owns ProtocolRegistry, ProtocolLink, and MavlinkTelemetryRegistry.
/// @note Protocol polling is handled externally.
/// @note MAVLink callback is configured in init().
struct ProtocolSystem : System<ProtocolSystem>, kf::mixin::Configurable<Config> {

    using kf::mixin::Configurable<Config>::Configurable;

    /// @brief Get mutable access to protocol link component
    protocol::ProtocolLink &link() noexcept {
        return _protocol_link;
    }

    /// @brief Get readonly access to protocol link component
    constexpr const protocol::ProtocolLink &link() const noexcept {
        return _protocol_link;
    }

    /// @brief Get mutable access to protocol registry component
    protocol::ProtocolRegistry &protocolRegistry() noexcept {
        return _protocol_registry;
    }

    /// @brief Get readonly access to protocol registry component
    constexpr const protocol::ProtocolRegistry &protocolRegistry() const noexcept {
        return _protocol_registry;
    }

    /// @brief Get mutable access to MAVLink telemetry registry component
    MavlinkTelemetryRegistry &mavlinkTelemetryRegistry() noexcept {
        return _mavlink_telemetry_registry;
    }

    /// @brief Get readonly access to MAVLink telemetry registry component
    constexpr const MavlinkTelemetryRegistry &mavlinkTelemetryRegistry() const noexcept {
        return _mavlink_telemetry_registry;
    }

private:
    static constexpr auto logger{kf::Logger::create("ProtocolSystem")};

    MavlinkTelemetryRegistry _mavlink_telemetry_registry{};
    protocol::ProtocolRegistry _protocol_registry{this->config().protocol_registry};
    protocol::ProtocolLink _protocol_link{this->config().protocol_link};
    kf::math::Milliseconds _poll_time{};

    KF_IMPL_INITABLE(ProtocolSystem, bool);
    bool initImpl() noexcept {
        _protocol_registry.mavlink().callback([this](const auto &message) {
            _mavlink_telemetry_registry.update(static_cast<kf::math::Milliseconds>(_poll_time), message);
        });

        _protocol_link.protocol(_protocol_registry.get(this->config().init_protocol_mode));

        return true;
    }

    KF_IMPL_TIMED_POLLABLE(ProtocolSystem);
    void poll(kf::math::Milliseconds now) noexcept {
        _poll_time = now;

        // protocol link polling in control component
    }
};

}// namespace djc::system
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <WiFi.h>

#include <kf/Logger.hpp>
#include <kf/mixin/Configurable.hpp>

#include "djc/Config.hpp"
#include "djc/system/System.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/transport/TransportRegistry.hpp"

namespace djc::system {

/// @brief System managing transport, Wraps TransportRegistry and TransportLink
/// @note Initializes WiFi STA mode, and polls the link for connection timeouts.
struct TransportSystem : System<TransportSystem>, kf::mixin::Configurable<Config> {

    using kf::mixin::Configurable<Config>::Configurable;

    /// @brief Get mutable access to transport link component
    transport::TransportLink &link() noexcept {
        return _transport_link;
    }

    /// @brief Get readonly access to transport link component
    constexpr const transport::TransportLink &link() const noexcept {
        return _transport_link;
    }

    /// @brief Get mutable access to transport registry component
    transport::TransportRegistry &registry() noexcept {
        return _transport_registry;
    }

    /// @brief Get readonly access to transport registry component
    constexpr const transport::TransportRegistry &registry() const noexcept {
        return _transport_registry;
    }

private:
    static constexpr auto logger{kf::Logger::create("TransportSystem")};

    transport::TransportRegistry _transport_registry{};
    transport::TransportLink _transport_link{this->config().transport_link};

    KF_IMPL_INITABLE(TransportSystem, bool);
    bool initImpl() noexcept {
        WiFi.mode(WIFI_MODE_STA);

        if (not _transport_registry.espnow().init()) {
            logger.error("failed to initialize espnow transport");
        }

        _transport_link.transport(_transport_registry.get(this->config().init_transport_kind));

        return true;
    }

    KF_IMPL_TIMED_POLLABLE(TransportSystem);
    void poll(kf::math::Milliseconds now) noexcept {
        _transport_link.poll(now);
    }
};

}// namespace djc::system
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/service/ConfigService.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System owning the config service, handling deferred NVS operations
/// @note Wraps ConfigService, requests load on init, and polls it periodically
struct ConfigSystem : System<ConfigSystem> {

    /// @brief Get mutable access to config service component
    service::ConfigService &configService() noexcept {
        return _config_service;
    }

    /// @brief Get readonly access to config service component
    constexpr const service::ConfigService &configService() const noexcept {
        return _config_service;
    }

private:
    service::ConfigService _config_service{};

    KF_IMPL_INITABLE(ConfigSystem, bool);
    bool initImpl() noexcept {
        _config_service.requestLoad();

        return true;
    }

    KF_IMPL_TIMED_POLLABLE(ConfigSystem);
    void poll(kf::math::Milliseconds now) noexcept {
        _config_service.poll(now);
    }
};

}// namespace djc::system
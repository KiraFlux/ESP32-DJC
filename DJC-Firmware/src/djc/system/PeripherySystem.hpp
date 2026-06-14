// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>

#include "djc/Periphery.hpp"
#include "djc/service/ConfigService.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System that owns and initializes hardware peripherals
/// @note Provides access to peripherals for other systems via getters; no periodic polling needed.
struct PeripherySystem : System<PeripherySystem> {

    explicit PeripherySystem(service::ConfigService &config_service) noexcept :
        _config_service{config_service} {}

    /// @brief Get mutable access to periphery component
    Periphery &periphery() noexcept {
        return _periphery;
    }

    /// @brief Get readonly access to periphery component
    constexpr const Periphery &periphery() const noexcept {
        return _periphery;
    }

private:
    static constexpr auto logger{kf::Logger::create("PeripherySystem")};

    service::ConfigService &_config_service;
    Periphery _periphery{_config_service.config().periphery};

    KF_IMPL_INITABLE(PeripherySystem, bool);
    bool initImpl() noexcept {
        if (not _periphery.init()) {
            logger.error("Periphery init failed. Resseting periphery config to defaults");
            _config_service.config().periphery = djc::Periphery::Config::defaults();
            _config_service.requestSave();

            return false;
        }

        if (not _config_service.config().periphery.joystick_axes_tuned) {
            logger.debug("Tunning axes..");
            _periphery.tune(_config_service.config().periphery);
            _config_service.requestSave();
        }

        return true;
    }

    KF_IMPL_TIMED_POLLABLE(PeripherySystem);
    void poll(kf::math::Milliseconds now) noexcept {}
};

}// namespace djc::system
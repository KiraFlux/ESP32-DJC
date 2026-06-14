// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/mixin/Configurable.hpp>

#include "djc/Config.hpp"
#include "djc/Periphery.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System that owns and initializes hardware peripherals
/// @note Provides access to peripherals for other systems via getters; no periodic polling needed.
struct PeripherySystem : System<PeripherySystem>, kf::mixin::Configurable<Config> {

    using kf::mixin::Configurable<Config>::Configurable;

    /// @brief Get mutable access to periphery component
    Periphery &periphery() noexcept {
        return _periphery;
    }

    /// @brief Get readonly access to periphery component
    constexpr const Periphery &periphery() const noexcept {
        return _periphery;
    }

    /// @brief Tune periphery
    void tune(Periphery::Config &mutable_periphery_config) noexcept {
        if (mutable_periphery_config.joystick_axes_tuned) {
            logger.debug("Nothing to do: axes already tuned");
        } else {
            logger.debug("Tunning axes..");
            _periphery.tune(mutable_periphery_config);
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("PeripherySystem")};

    Periphery _periphery{this->config().periphery};

    KF_IMPL_INITABLE(PeripherySystem, bool);
    bool initImpl() noexcept {
        if (_periphery.init()) {
            return true;
        } else {
            logger.error("Periphery init failed");
            return false;
        }
    }

    KF_IMPL_TIMED_POLLABLE(PeripherySystem);
    void poll(kf::math::Milliseconds now) noexcept {}
};

}// namespace djc::system
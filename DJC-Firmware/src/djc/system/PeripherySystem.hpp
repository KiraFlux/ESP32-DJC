// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>

#include "djc/Config.hpp"
#include "djc/Periphery.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System that owns and initializes hardware peripherals
/// @note Provides access to peripherals for other systems via getters; no periodic polling needed.
struct PeripherySystem : System<PeripherySystem, bool()> {

    explicit PeripherySystem(const Config &config) noexcept :
        _periphery{config.periphery} {}

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

    Periphery _periphery;

    DJC_IMPL_INITABLE(PeripherySystem, bool());
    bool initImpl() noexcept {
        if (_periphery.init()) {
            return true;
        } else {
            logger.error("Periphery init failed");
            return false;
        }
    }

    KF_IMPL_TIMED_POLLABLE(PeripherySystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {}
};

}// namespace djc::system
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/Periphery.hpp"
#include "djc/config/DeviceConfig.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System that owns and initializes hardware peripherals
/// @note Provides access to peripherals for other systems via getters; no periodic polling needed.
struct PeripherySystem : System<PeripherySystem, void()> {

    explicit PeripherySystem(const config::DeviceConfig &config) noexcept :
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
    Periphery _periphery;

    KF_IMPL_INITABLE(PeripherySystem, void());
    void initImpl() noexcept {
        _periphery.init();
    }

    KF_IMPL_TIMED_POLLABLE(PeripherySystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {}
};

}// namespace djc::system
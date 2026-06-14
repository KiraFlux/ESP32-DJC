// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/mixin/Configurable.hpp>

#include "djc/Config.hpp"
#include "djc/Periphery.hpp"
#include "djc/service/InputHandler.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System managing user input
/// @note Owns InputHandler service. Depends on Config (read‑only) and Periphery.
struct InputSystem : System<InputSystem>, kf::mixin::Configurable<Config> {

    explicit InputSystem(const Config &config, Periphery &periphery) noexcept :
        kf::mixin::Configurable<Config>{config},
        _input_handler{
            config.input_handler,
            periphery.right_joystick,// primary joystick for navigation/control
            periphery.left_button_listener,
            periphery.right_button_listener,
        } {}

    /// @brief Get mutable access to the input handler service (for callback setup).
    service::InputHandler &inputHandler() noexcept {
        return _input_handler;
    }

    /// @brief Get readonly access to the input handler service.
    constexpr const service::InputHandler &inputHandler() const noexcept {
        return _input_handler;
    }

private:
    service::InputHandler _input_handler;

    KF_IMPL_INITABLE(InputSystem, bool);
    bool initImpl() noexcept {
        // For now, nothing to do.
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(InputSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _input_handler.poll(now);
    }
};

}// namespace djc::system
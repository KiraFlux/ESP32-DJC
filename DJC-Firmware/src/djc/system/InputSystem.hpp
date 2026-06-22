// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/Config.hpp"
#include "djc/Periphery.hpp"
#include "djc/mixin/ServiceOwner.hpp"
#include "djc/service/InputHandler.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System managing user input
/// @note Owns InputHandler service. Depends on Config (read‑only) and Periphery.
struct InputSystem :

    System<InputSystem, void()>,
    mixin::ServiceOwner<service::InputHandler>

{
    explicit InputSystem(const Config &config, Periphery &periphery) noexcept :
        mixin::ServiceOwner<service::InputHandler>{service::InputHandler{
            config.input_handler,
            periphery.right_joystick,// primary joystick for navigation/control
            periphery.left_button_listener,
            periphery.right_button_listener,
        }} {}

private:
    KF_IMPL_INITABLE(InputSystem, void());
    void initImpl() noexcept {}

    KF_IMPL_TIMED_POLLABLE(InputSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
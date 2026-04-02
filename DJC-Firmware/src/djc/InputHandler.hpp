// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/Function.hpp>
#include <kf/aliases.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/DeviceState.hpp"
#include "djc/Periphery.hpp"

namespace djc {

struct InputHandler final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<InputHandler> {

    /// @brief Controller values for manual control mode
    struct ControllerValues {
        kf::f32 left_x{};
        kf::f32 left_y{};
        kf::f32 right_x{};
        kf::f32 right_y{};

        void reset() noexcept { *this = ControllerValues{}; }
    };

    using ClickCallback = kf::Function<void()>;
    using DirectionCallback = kf::Function<void(JoystickListener::Direction)>;

    explicit InputHandler(Periphery &periphery, const DeviceState &device_state) noexcept :
        _periphery{periphery}, _device_state{device_state} {}

    const ControllerValues &controllerValues() const noexcept { return _controller_values; }

    void onRightButton(ClickCallback &&callback) noexcept { _on_right_button_click = std::move(callback); }

    void onLeftButton(ClickCallback &&callback) noexcept { _on_left_button_click = std::move(callback); }

    void onDirection(DirectionCallback &&callback) noexcept { _on_joystick_direction = std::move(callback); }

private:
    Periphery &_periphery;
    const DeviceState &_device_state;
    ControllerValues _controller_values{};
    ClickCallback _on_right_button_click{};
    ClickCallback _on_left_button_click{};
    DirectionCallback _on_joystick_direction{};

    // impl

    KF_IMPL_TIMED_POLLABLE(InputHandler);
    void pollImpl(kf::math::Milliseconds now) noexcept {

        if (_on_left_button_click) {
            _periphery.left_button.poll(now);
            if (_periphery.left_button.clicked()) { _on_left_button_click(); }
        }

        if (_device_state.menu_navigation_enabled) {
            if (_on_right_button_click) {
                _periphery.right_button.poll(now);
                if (_periphery.right_button.clicked()) { _on_right_button_click(); }
            }

            if (_on_joystick_direction) {
                _periphery.right_joystick_listener.poll(now);
                const auto direction = _periphery.right_joystick_listener.direction();
                if (direction != JoystickListener::Direction::Home and _periphery.right_joystick_listener.changed()) {
                    _on_joystick_direction(direction);
                }
            }
        } else {
            _controller_values.left_x = _periphery.left_joystick.axis_x.read();
            _controller_values.left_y = _periphery.left_joystick.axis_y.read();
            _controller_values.right_x = _periphery.right_joystick.axis_x.read();
            _controller_values.right_y = _periphery.right_joystick.axis_y.read();
        }
    }
};

}// namespace djc
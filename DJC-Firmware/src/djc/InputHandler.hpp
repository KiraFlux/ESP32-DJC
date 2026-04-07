// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/Function.hpp>
#include <kf/aliases.hpp>
#include <kf/input/JoystickListener.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/Resettable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/DeviceState.hpp"
#include "djc/Periphery.hpp"

namespace djc {

struct InputHandler final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<InputHandler> {
    using JoystickListener = kf::input::JoystickListener<Joystick>;
    using ClickCallback = kf::Function<void()>;
    using DirectionCallback = kf::Function<void(JoystickListener::Direction)>;

    struct Config {
        JoystickListener::Config joystick_listener;

        static constexpr Config defaults() noexcept {
            return Config{
                .joystick_listener = JoystickListener::Config{
                    .threshold = 0.6f,
                    .repeat_timeout = 100,// ms
                    .delay = 400,         // ms
                },
            };
        }
    };

    /// @brief Controller values for manual control mode
    struct ControllerValues final : kf::mixin::NonCopyable, kf::mixin::Resettable<ControllerValues> {
        using Unit = kf::f32;

        kf::f32 left_x;
        kf::f32 left_y;
        kf::f32 right_x;
        kf::f32 right_y;

    private:
        KF_IMPL_RESETTABLE(ControllerValues);
        void resetImpl() noexcept { left_x = left_y = right_x = right_y = Unit{}; }
    };

    explicit InputHandler(Periphery &periphery, const DeviceState &device_state, const Config &config) noexcept :
        _periphery{periphery}, _device_state{device_state}, _joystick_listener{periphery.right_joystick, config.joystick_listener} {}

    const ControllerValues &controllerValues() const noexcept { return _controller_values; }

    void onRightButton(ClickCallback &&callback) noexcept { _right_click_callback = std::move(callback); }

    void onLeftButton(ClickCallback &&callback) noexcept { _left_click_callback = std::move(callback); }

    void onDirection(DirectionCallback &&callback) noexcept { _direction_callback = std::move(callback); }

private:
    Periphery &_periphery;
    const DeviceState &_device_state;
    JoystickListener _joystick_listener;
    ControllerValues _controller_values{};
    ClickCallback _right_click_callback{};
    ClickCallback _left_click_callback{};
    DirectionCallback _direction_callback{};

    // impl

    KF_IMPL_TIMED_POLLABLE(InputHandler);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_left_click_callback) {
            _periphery.left_button.poll(now);
            if (_periphery.left_button.clicked()) { _left_click_callback(); }
        }

        if (_device_state.controlEnabled()) {
            _controller_values.left_x = _periphery.left_joystick.axis_x.read();
            _controller_values.left_y = _periphery.left_joystick.axis_y.read();
            _controller_values.right_x = _periphery.right_joystick.axis_x.read();
            _controller_values.right_y = _periphery.right_joystick.axis_y.read();
        } else {
            _controller_values.reset();// todo make once with flag

            if (_right_click_callback) {
                _periphery.right_button.poll(now);
                if (_periphery.right_button.clicked()) { _right_click_callback(); }
            }

            if (_direction_callback) {
                _joystick_listener.poll(now);
                const auto direction = _joystick_listener.direction();
                if (direction != JoystickListener::Direction::Home and _joystick_listener.changed()) {
                    _direction_callback(direction);
                }
            }
        }
    }
};

}// namespace djc
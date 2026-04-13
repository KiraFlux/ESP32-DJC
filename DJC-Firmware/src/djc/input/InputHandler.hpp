// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/Function.hpp>
#include <kf/aliases.hpp>
#include <kf/input/JoystickListener.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/prelude.hpp"

namespace djc {

struct InputHandler final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<InputHandler> {
    using JoystickListener = kf::input::JoystickListener<Joystick>;

    using ClickCallback = kf::Function<void()>;
    using DirectionCallback = kf::Function<void(JoystickListener::Direction)>;

    struct Config final : kf::mixin::NonCopyable {
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

    explicit InputHandler(
        const Config &config,
        Joystick &primary_joystick,
        ButtonListener &left_button_listener,
        ButtonListener &right_button_listener) noexcept :
        _joystick_listener{primary_joystick, config.joystick_listener},
        _left_button_listener{left_button_listener},
        _right_button_listener{right_button_listener} {}

    void onRightButton(ClickCallback &&callback) noexcept { _right_click_callback = std::move(callback); }

    void onLeftButton(ClickCallback &&callback) noexcept { _left_click_callback = std::move(callback); }

    void onDirection(DirectionCallback &&callback) noexcept { _direction_callback = std::move(callback); }

private:
    JoystickListener _joystick_listener;
    DirectionCallback _direction_callback{};

    ButtonListener &_left_button_listener;
    ClickCallback _left_click_callback{};

    ButtonListener &_right_button_listener;
    ClickCallback _right_click_callback{};

    // impl

    KF_IMPL_TIMED_POLLABLE(InputHandler);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _left_button_listener.poll(now);
        if (_left_click_callback and _left_button_listener.clicked()) {
            _left_click_callback();
        }

        _right_button_listener.poll(now);
        if (_right_click_callback and _right_button_listener.clicked()) {
            _right_click_callback();
        }

        _joystick_listener.poll(now);
        if (_direction_callback and (_joystick_listener.direction() != JoystickListener::Direction::Home) and _joystick_listener.changed()) {
            _direction_callback(_joystick_listener.direction());
        }
    }
};

}// namespace djc
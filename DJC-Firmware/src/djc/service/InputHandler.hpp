// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <utility>

#include <kf/Function.hpp>
#include <kf/NoneType.hpp>
#include <kf/Option.hpp>
#include <kf/input/JoystickListener.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Resettable.hpp>

#include "djc/Periphery.hpp"
#include "djc/service/Service.hpp"

namespace djc::service {

// TODO: make implements 3x callbacked
struct InputHandler : Service<InputHandler> {
    using JoystickListener = kf::input::JoystickListener<Periphery::Joystick>;

    using ClickCallback = kf::Function<void()>;
    using DirectionCallback = kf::Function<void(JoystickListener::Direction)>;

    struct Config : kf::mixin::Resettable<Config> {
        JoystickListener::Config joystick_listener;

    private:
        KF_IMPL_RESETTABLE(Config);
        void resetImpl() noexcept {
            joystick_listener.repeat_timer.period = 100;// ms
            joystick_listener.delay_timer.period = 400; // ms
            joystick_listener.threshold = 0.6f;
        }
    };

    explicit InputHandler(const Config &config, Periphery &periphery) noexcept :
        _joystick_listener{periphery.right_joystick, config.joystick_listener},
        _left_button_listener{periphery.left_button_listener},
        _right_button_listener{periphery.right_button_listener} {}

    template<typename F> void onRightButton(F &&callback) noexcept {
        _right_click_callback = kf::some(ClickCallback{std::forward<F>(callback)});
    }

    void onRightButton(kf::NoneType) {
        _right_click_callback.reset();
    }

    template<typename F> void onLeftButton(F &&callback) noexcept {
        _left_click_callback = kf::some(ClickCallback{std::forward<F>(callback)});
    }

    void onLeftButton(kf::NoneType) {
        _left_click_callback.reset();
    }

    template<typename F> void onDirection(F &&callback) noexcept {
        _direction_callback = kf::some(DirectionCallback{std::forward<F>(callback)});
    }

    void onDirection(kf::NoneType) noexcept {
        _direction_callback.reset();
    }

private:
    JoystickListener _joystick_listener;
    Periphery::ButtonListener &_left_button_listener, &_right_button_listener;

    kf::Option<DirectionCallback> _direction_callback{kf::none};
    kf::Option<ClickCallback> _left_click_callback{kf::none}, _right_click_callback{kf::none};

    KF_IMPL_TIMED_POLLABLE(InputHandler);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _left_button_listener.poll(now);
        if (_left_click_callback.isSome() and _left_button_listener.clicked()) {
            _left_click_callback.unwrap()();
        }

        _right_button_listener.poll(now);
        if (_right_click_callback.isSome() and _right_button_listener.clicked()) {
            _right_click_callback.unwrap()();
        }

        _joystick_listener.poll(now);
        if (_direction_callback.isSome() and (_joystick_listener.direction() != JoystickListener::Direction::Home) and _joystick_listener.changed()) {
            _direction_callback.unwrap()(_joystick_listener.direction());
        }
    }
};

}// namespace djc::service
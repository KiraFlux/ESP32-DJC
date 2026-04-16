// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <utility>

#include <Arduino.h>// for delay

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/prelude.hpp"

namespace djc {

namespace internal {

struct PeripheryConfig final : kf::mixin::NonCopyable {
    ButtonListener::Config button;

    AxisInput::FilterImpl::Config axis_filter;
    Joystick::Config left_joystick, right_joystick;

    Bus::Config bus;
    Bus::Node::Config bus_node;

    DisplayDriver::Config display;
    kf::u16 joystick_axes_tune_samples;
    bool joystick_axes_tuned;

    static constexpr PeripheryConfig defaults() noexcept {
        return PeripheryConfig{
            .button = {
                .debounce = 50,// ms
            },
            .axis_filter = {
                .factor = 0.5f,
            },
            .left_joystick = {
                .x = axisDefaults(true),
                .y = axisDefaults(false),
            },
            .right_joystick = {
                .x = axisDefaults(false),
                .y = axisDefaults(true),
            },
            // SPI default pins: MOSI=23, MISO=19, SCK=18
            .bus = djc::Bus::Config::create(),
            // CS, SPI frequency
            .bus_node = djc::Bus::Node::Config::create(GPIO_NUM_5, 27000000),
            .display = {
                .init_orientation = kf::drivers::display::Orientation::ClockWise,
            },
            .joystick_axes_tune_samples = 100,
            .joystick_axes_tuned = false,
        };
    }

private:
    static constexpr AxisInput::Config axisDefaults(bool inverted) noexcept {
        return AxisInput::Config{
            .inverted = inverted,
            .dead_zone = 200,
            .range_positive = 2000,
            .range_negative = 2000,
        };
    }
};

}// namespace internal

/// @brief ESP32-DJC Hardware Periphery
struct Periphery final : kf::mixin::NonCopyable, kf::mixin::Initable<Periphery, bool>, kf::mixin::Configurable<internal::PeripheryConfig> {
    using Config = internal::PeripheryConfig;

    using Configurable<Config>::Configurable;

    ButtonListener left_button_listener{
        this->config().button,
        DigitalInput{
            GPIO_NUM_14,
            DigitalInput::Pull::InternalUp,
        },
    };

    Joystick left_joystick{
        this->config().left_joystick,
        this->config().axis_filter,
        AdcInput{GPIO_NUM_32},
        AdcInput{GPIO_NUM_33},
    };

    ButtonListener right_button_listener{
        this->config().button,
        DigitalInput{
            GPIO_NUM_4,
            DigitalInput::Pull::InternalUp,
        },
    };

    Joystick right_joystick{
        this->config().right_joystick,
        this->config().axis_filter,
        AdcInput{GPIO_NUM_34},
        AdcInput{GPIO_NUM_35},
    };

    Bus bus{
        this->config().bus,
        SPI,
    };

    DisplayDriver display{
        this->config().display,
        bus.createNode(this->config().bus_node),
        DigitalOutput{GPIO_NUM_22},// DC
        DigitalOutput{GPIO_NUM_17},// RESET
    };

    // Analog axis calibration
    void tune(Config &mut_config) noexcept {
        Joystick::Tuner left_tuner{mut_config.left_joystick, left_joystick, mut_config.joystick_axes_tune_samples};
        Joystick::Tuner right_tuner{mut_config.right_joystick, right_joystick, mut_config.joystick_axes_tune_samples};

        left_tuner.reset();
        right_tuner.reset();

        // Poll all axes until calibration complete
        while (left_tuner.running() or right_tuner.running()) {
            left_tuner.poll();
            right_tuner.poll();
            delay(1);
        }

        mut_config.joystick_axes_tuned = true;
    }

private:
    static constexpr auto logger = kf::Logger::create("Periphery");

    // impl
    KF_IMPL_INITABLE(Periphery, bool);
    bool initImpl() noexcept {
        logger.info("Initializing peripherals");

        left_joystick.init();
        right_joystick.init();
        left_button_listener.init();
        right_button_listener.init();

        if (bus.init().isError()) {
            logger.error("Bus initialization failed");
        }

        if (not display.init()) {
            logger.error("Display driver initialization failed");
        }

        logger.info("Peripherals initialized successfully");
        return true;
    }
};

}// namespace djc
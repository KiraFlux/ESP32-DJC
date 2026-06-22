// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Arduino.h>// for delay

#include <kf/Logger.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/primitives.hpp>

#include "djc/prelude.hpp"

namespace djc {

namespace internal {

struct PeripheryConfig final {
    ButtonListener::Config button;

    AxisInput::FilterImpl::Config axis_filter;
    Joystick::Config left_joystick, right_joystick;

    SpiBus::Config spi_bus;
    SpiBus::Node::Config display_spi_node;

    DisplayDriver::Config display_driver;
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
            .spi_bus = djc::SpiBus::Config::create(gpio_spi_mosi, gpio_spi_miso, gpio_spi_sck),
            .display_spi_node = djc::SpiBus::Node::Config::create(gpio_display_st7735_spi_cs, 27000000),
            .display_driver = {
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
struct Periphery final :

    kf::mixin::NonCopyable,
    kf::mixin::Initable<Periphery, void()>,
    kf::mixin::Configurable<internal::PeripheryConfig>

{
    using Config = internal::PeripheryConfig;

    using Configurable<Config>::Configurable;

    ButtonListener left_button_listener{
        this->config().button,
        GPIO::DigitalInput{
            gpio_button_left,
            GPIO::DigitalInput::Pull::InternalUp,
        },
    };

    Joystick left_joystick{
        this->config().left_joystick,
        this->config().axis_filter,
        GPIO::AdcInput{gpio_joystick_left_x},
        GPIO::AdcInput{gpio_joystick_left_y},
    };

    ButtonListener right_button_listener{
        this->config().button,
        GPIO::DigitalInput{
            gpio_button_right,
            GPIO::DigitalInput::Pull::InternalUp,
        },
    };

    Joystick right_joystick{
        this->config().right_joystick,
        this->config().axis_filter,
        GPIO::AdcInput{gpio_joystick_right_x},
        GPIO::AdcInput{gpio_joystick_right_y},
    };

    SpiBus spi_bus{
        this->config().spi_bus,
        SPI,
    };

    DisplayDriver display_driver{
        this->config().display_driver,
        spi_bus.createNode(this->config().display_spi_node),
        GPIO::DigitalOutput{gpio_display_st7735_data_command},
        GPIO::DigitalOutput{gpio_display_st7735_reset},
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

    KF_IMPL_INITABLE(Periphery, void());
    void initImpl() noexcept {
        logger.info("Initializing peripherals");

        left_joystick.init();
        right_joystick.init();
        left_button_listener.init();
        right_button_listener.init();

        if (spi_bus.init().isError()) {
            logger.error("SpiBus initialization failed");
        }

        if (display_driver.init().isError()) {
            logger.error("Display driver initialization failed");
        }
    }
};

}// namespace djc
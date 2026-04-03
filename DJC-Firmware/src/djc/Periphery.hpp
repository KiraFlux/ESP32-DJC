// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/prelude.hpp"

namespace djc {

namespace internal {

struct PeripheryConfig final : kf::mixin::NonCopyable {
    Button::Config button;

    AxisInput::FilterImpl::Config axis_filter;
    Joystick::Config left_joystick, right_joystick;

    Bus::Config bus;
    Bus::Node::Config bus_node;

    DisplayDriver::Config display;
};

}// namespace internal

/// @brief ESP32-DJC Hardware Periphery
struct Periphery final : kf::mixin::NonCopyable, kf::mixin::Initable<Periphery, bool>, kf::mixin::Configurable<internal::PeripheryConfig> {
    using Config = internal::PeripheryConfig;

    using Configurable<Config>::Configurable;

    Button left_button{
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

    Button right_button{
        this->config().button,
        DigitalInput{
            GPIO_NUM_4,
            DigitalInput::Pull::InternalUp,
        },
    };

    Joystick right_joystick{
        this->config().right_joystick,
        this->config().axis_filter,
        AdcInput{GPIO_NUM_35},
        AdcInput{GPIO_NUM_34},
    };

    Bus bus{
        this->config().bus,
        SPI,
    };

    DisplayDriver display{
        this->config().display,
        bus.createNode(this->config().bus_node),
        DigitalOutput{GPIO_NUM_2}, // DC
        DigitalOutput{GPIO_NUM_15},// RESET
    };

    // Analog axis calibration
    void tune(Config &mut_config, kf::u16 samples) noexcept {
        Joystick::Tuner left_tuner{mut_config.left_joystick, left_joystick, samples};
        Joystick::Tuner right_tuner{mut_config.right_joystick, right_joystick, samples};

        left_tuner.reset();
        right_tuner.reset();

        // Poll all axes until calibration complete
        while (left_tuner.running() or right_tuner.running()) {
            left_tuner.poll();
            right_tuner.poll();
            delay(1);
        }
    }

private:
    static constexpr auto logger = kf::Logger::create("Periphery");

    // impl
    KF_IMPL_INITABLE(Periphery, bool);
    bool initImpl() noexcept {
        logger.info("Initializing peripherals");

        left_joystick.init();
        right_joystick.init();
        left_button.init();
        right_button.init();

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
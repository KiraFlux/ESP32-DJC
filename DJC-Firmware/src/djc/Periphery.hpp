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

struct PeripheryConfig {
    // ESPNOW peer MAC address (broadcast by default)
    EspNow::Mac peer_mac{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // Input devices configuration
    Button::Config button{
        .debounce = 50,
    };

    AxisInput::FilterImpl::Config axis_filter{
        .factor = 0.5f,
    };

    Joystick::Config left_joystick{
        .x = {.inverted = true},
        .y = {.inverted = false},
    };

    Joystick::Config right_joystick{
        .x = {.inverted = false},
        .y = {.inverted = true},
    };

    JoystickListener::Config joystick_listener{
        .threshold = 0.6f,
        .repeat_timeout = 100,
        .delay = 400,
    };

    // Display configuration

    Bus::Config bus{};// defaults

    Bus::Node::Config bus_node{
        GPIO_NUM_5,// CS
        27000000,  // SPI frequency
    };

    DisplayDriver::Config display{
        .init_orientation = kf::drivers::display::Orientation::ClockWise,
    };
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

    JoystickListener right_joystick_listener{right_joystick, this->config().joystick_listener};

    kf::Option<EspNow::Peer> espnow_peer{};

    Bus bus{
        this->config().bus,
        SPI,
    };

    DisplayDriver display{
        this->config().display,
        bus.createNode(this->config().bus_node),
        DigitalOutput{GPIO_NUM_2}, // DC
        DigitalOutput{GPIO_NUM_15},// RESET
        // SPI pins: MOSI=23, MISO=19, SCK=18
    };

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

        const auto espnow_init_result = EspNow::instance().init();
        if (espnow_init_result.isError()) {
            logger.error("Failed to initialize ESP-NOW: %s");
            return false;
        }

        auto peer_result = EspNow::Peer::add(this->config().peer_mac);
        if (peer_result.isError()) {
            logger.error("ESPNOW failed to add peer");
            return false;
        }

        espnow_peer.value(std::move(peer_result.value()));
        logger.info("Peripherals initialized successfully");
        return true;
    }
};

}// namespace djc
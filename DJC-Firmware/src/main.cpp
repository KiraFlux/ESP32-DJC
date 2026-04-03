// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Control.hpp"
#include "djc/DeviceConfig.hpp"
#include "djc/DeviceState.hpp"
#include "djc/DisplayManager.hpp"
#include "djc/InputHandler.hpp"
#include "djc/Periphery.hpp"
#include "djc/UiManager.hpp"

static djc::DeviceState device_state{
    .menu_navigation_enabled = true,
};

static djc::DeviceConfig device_config{
    .periphery = {
        .button = {
            .debounce = 50,// ms
        },
        .axis_filter = {
            .factor = 0.5f,
        },
        .left_joystick = {
            .x = {.inverted = true},
            .y = {.inverted = false},
        },
        .right_joystick = {
            .x = {.inverted = false},
            .y = {.inverted = true},
        },
        .bus = djc::Bus::Config{
            // defaults
            // SPI pins: MOSI=23, MISO=19, SCK=18
        },
        .bus_node = djc::Bus::Node::Config{
            GPIO_NUM_5,// CS
            27000000,  // SPI frequency
        },
        .display = {
            .init_orientation = kf::drivers::display::Orientation::ClockWise,
        },
    },
    .input_handler = {
        .joystick_listener = {
            .threshold = 0.6f,
            .repeat_timeout = 100,// ms
            .delay = 400,         // ms
        },
    },
    .control = {
        .heartbeat_period = 2000,                                     // ms
        .poll_period = static_cast<kf::math::Milliseconds>(1000 / 50),// 50 Hz
        .mode = djc::Control::Mode::Raw,
    },
    .favorites = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},// broadcast
    },
    .active_favorite_index = 0,// select broadcast as default
};

static djc::Periphery periphery{device_config.periphery};

static djc::InputHandler input_handler{periphery, device_state, device_config.input_handler};

static djc::Control control{device_config.control, device_state, input_handler};

static djc::DisplayManager display_manager{periphery.display, device_state};

static djc::UiManager ui_manager{};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    (void) periphery.init();
    periphery.tune(device_config.periphery, 100);

    {
        using E = djc::ui::UI::Event;

        input_handler.onLeftButton([]() {
            device_state.menu_navigation_enabled ^= 1;// toggle
            ui_manager.addEvent(E::update());
        });

        input_handler.onRightButton([]() {
            ui_manager.addEvent(E::widgetClick());
        });

        input_handler.onDirection([](djc::InputHandler::JoystickListener::Direction direction) {
            static constexpr E event_from_direction[4] = {
                E::pageCursorMove(-1),// Up
                E::pageCursorMove(+1),// Down
                E::widgetValue(-1),   // Left
                E::widgetValue(+1),   // Right
            };

            ui_manager.addEvent(event_from_direction[static_cast<kf::u8>(direction)]);
        });
    }

    display_manager.init();
    ui_manager.init();
    (void) control.init();
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    input_handler.poll(now);
    control.poll(now);
    ui_manager.poll(now);
}
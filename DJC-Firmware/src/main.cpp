// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Control.hpp"
#include "djc/DeviceConfig.hpp"
#include "djc/DeviceState.hpp"
#include "djc/DisplayManager.hpp"
#include "djc/InputHandler.hpp"
#include "djc/Periphery.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

static constexpr auto logger{kf::Logger::create("root")};

static auto &ui{djc::ui::UI::instance()};

static djc::DeviceState device_state{
    .menu_navigation_enabled = true,
};

static kf::memory::Storage<djc::DeviceConfig> storage{
    .key = "DC",
    .config = djc::DeviceConfig::defaults(),
};

// services

static djc::Periphery periphery{
    storage.config.periphery,
};

static djc::InputHandler input_handler{
    periphery,
    device_state,
    storage.config.input_handler,
};

static djc::Control control{
    storage.config.control,
    device_state,
    input_handler,
};

static djc::DisplayManager display_manager{
    periphery.display,
    device_state,
};

// pages

static djc::ui::pages::RootPage root{};

static djc::ui::pages::MavLinkControlPage mav_link_control{
    root,
    control,
};

static djc::ui::pages::ConfigPage config{
    root,
    storage,
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    if (not storage.load()) {
        logger.warn("failed to load device config. Using defaults");
        storage.config = djc::DeviceConfig::defaults();

        if (not storage.save()) {
            logger.error("failed to save defaults");
        }
    }

    if (not periphery.init()) {
        logger.error("Periphery init failed. Resseting periphery config to defaults");
        storage.config.periphery = djc::Periphery::Config::defaults();

        (void) storage.save();
    }

    using E = djc::ui::UI::Event;

    {
        input_handler.onLeftButton([]() {
            device_state.menu_navigation_enabled ^= 1;// toggle
            ui.addEvent(E::update());
        });

        input_handler.onRightButton([]() {
            ui.addEvent(E::widgetClick());
        });

        input_handler.onDirection([](djc::InputHandler::JoystickListener::Direction direction) {
            static constexpr E event_from_direction[4] = {
                E::pageCursorMove(-1),// Up
                E::pageCursorMove(+1),// Down
                E::widgetValue(-1),   // Left
                E::widgetValue(+1),   // Right
            };

            ui.addEvent(event_from_direction[static_cast<kf::u8>(direction)]);
        });
    }

    if (not storage.config.periphery.joystick_axes_tuned) {
        logger.debug("Need axes tune");
        periphery.tune(storage.config.periphery);
        logger.debug("Tune done!");

        (void) storage.save();
    } else {
        logger.debug("Axes already tuned");
    }

    (void) control.init();
    display_manager.init();

    {
        // apply page links
        // WARNING: before adding another link check RootPage::widget_layout LENGTH
        root.widget_layout[0] = &mav_link_control.link();
        root.widget_layout[1] = &config.link();
        ui.bindPage(root);
        ui.addEvent(E::update());
    }

    // (void) control.activePeer(storage.config.activePeer());
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    input_handler.poll(now);
    control.poll(now);
    ui.poll(now);
}
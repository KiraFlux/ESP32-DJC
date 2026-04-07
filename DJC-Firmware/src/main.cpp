// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/ConfigManager.hpp"
#include "djc/Control.hpp"
#include "djc/DeviceState.hpp"
#include "djc/DisplayManager.hpp"
#include "djc/InputHandler.hpp"
#include "djc/Periphery.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkPage.hpp"
#include "djc/ui/pages/PeerExplorerPage.hpp"
#include "djc/ui/pages/RawControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

static constexpr auto logger{kf::Logger::create("root")};

static auto &ui{djc::ui::UI::instance()};

static auto &storage{djc::ConfigManager::instance()};

static djc::DeviceState device_state{
    .mode = djc::DeviceState::Mode::UiNavigation,
};

// services

static djc::Periphery periphery{
    storage.config().periphery,
};

static djc::InputHandler input_handler{
    periphery,
    device_state,
    storage.config().input_handler,
};

static djc::Control control{
    storage.config().control,
    device_state,
    input_handler,
};

static djc::DisplayManager display_manager{
    periphery.display,
    device_state,
};

// pages

static djc::ui::pages::RootPage root_page{};

static djc::ui::pages::MavLinkPage mavlink_page{
    root_page,
    control,
};

static djc::ui::pages::RawControlPage raw_control_page{
    root_page,
    control,
};

static djc::ui::pages::PeerExplorerPage peer_explorer_page{
    root_page,
    control,
};

static djc::ui::pages::ConfigPage config_page{
    root_page,
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    storage.load();

    if (not periphery.init()) {
        logger.error("Periphery init failed. Resseting periphery config to defaults");
        storage.config().periphery = djc::Periphery::Config::defaults();
        storage.save();
    }

    using E = djc::ui::UI::Event;

    {
        input_handler.onLeftButton([]() {
            if (device_state.controlEnabled()) {
                device_state.mode = djc::DeviceState::Mode::UiNavigation;
            } else {
                device_state.mode = djc::DeviceState::Mode::Control;
            }

            ui.addEvent(E::update());
        });

        input_handler.onRightButton([]() {
            ui.addEvent(E::widgetClick());
        });

        input_handler.onDirection([](djc::InputHandler::JoystickListener::Direction direction) {
            static constexpr E navigation_event_from_direction[4] = {
                E::pageCursorMove(-1),// Up
                E::pageCursorMove(+1),// Down
                E::widgetValue(-1),   // Left
                E::widgetValue(+1),   // Right
            };

            ui.addEvent(navigation_event_from_direction[static_cast<kf::u8>(direction)]);
        });
    }

    if (not storage.config().periphery.joystick_axes_tuned) {
        logger.debug("Need axes tune");
        periphery.tune(storage.config().periphery);
        logger.debug("Tune done!");

        storage.save();
    } else {
        logger.debug("Axes already tuned");
    }

    (void) control.init();
    display_manager.init();

    {
        // apply page links
        root_page.attach(mavlink_page);
        root_page.attach(raw_control_page);
        root_page.attach(peer_explorer_page);
        root_page.attach(config_page);

        ui.bindPage(root_page);
        ui.addEvent(E::update());
    }
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    input_handler.poll(now);
    control.poll(now);
    ui.poll(now);
}
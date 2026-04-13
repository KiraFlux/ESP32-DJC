// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/ConfigManager.hpp"
#include "djc/Control.hpp"
#include "djc/DisplayManager.hpp"
#include "djc/Periphery.hpp"
#include "djc/input/InputHandler.hpp"
#include "djc/input/VirtualKeyboard.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkPage.hpp"
#include "djc/ui/pages/PeerExplorerPage.hpp"
#include "djc/ui/pages/RawControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

static auto &ui{djc::ui::UI::instance()};

static auto &storage{djc::ConfigManager::instance()};

static auto &virtual_keyboard{djc::input::VirtualKeyboard::instance()};

// services

static djc::Periphery periphery{
    storage.config().periphery,
};

static djc::InputHandler input_handler{
    storage.config().input_handler,
    periphery.right_joystick,
    periphery.left_button_listener,
    periphery.right_button_listener,
};

static djc::Control control{
    storage.config().control,
};

static djc::DisplayManager display_manager{
    periphery.display,
    control,
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
    bool config_modified{false};
    static constexpr auto logger{kf::Logger::create("setup")};

    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    storage.load();

    if (not periphery.init()) {
        logger.error("Periphery init failed. Resseting periphery config to defaults");
        storage.config().periphery = djc::Periphery::Config::defaults();
        config_modified = true;
    }

    if (not storage.config().periphery.joystick_axes_tuned) {
        logger.debug("Tunning axes..");
        periphery.tune(storage.config().periphery);
        config_modified = true;
    } else {
        logger.debug("Axes already tuned");
    }

    (void) control.init();// TODO: implement halt on error?
    display_manager.init();

    {
        using E = djc::ui::UI::Event;

        input_handler.onLeftButton([]() {
            if (virtual_keyboard.active()) {
                virtual_keyboard.quit();
            } else {
                control.enabled(not control.enabled());
            }

            ui.addEvent(E::update());
        });

        input_handler.onRightButton([]() {
            if (control.enabled()) { return; }

            ui.addEvent(E::widgetClick());
        });

        input_handler.onDirection([](djc::InputHandler::JoystickListener::Direction direction) {
            static constexpr E navigation_event_from_direction[4] = {
                E::pageCursorMove(-1),// Up
                E::pageCursorMove(+1),// Down
                E::widgetValue(-1),   // Left
                E::widgetValue(+1),   // Right
            };

            static constexpr E VirtualKeyboard_event_from_direction[4] = {
                E::widgetValue(0),// Up
                E::widgetValue(1),// Down
                E::widgetValue(2),// Left
                E::widgetValue(3),// Right
            };

            if (control.enabled()) { return; }

            const auto table = virtual_keyboard.active() ? VirtualKeyboard_event_from_direction : navigation_event_from_direction;
            ui.addEvent(table[static_cast<kf::u8>(direction)]);
        });

        // apply page links
        root_page.attach(mavlink_page);
        root_page.attach(raw_control_page);
        root_page.attach(peer_explorer_page);
        root_page.attach(config_page);

        ui.bindPage(root_page);
        ui.addEvent(E::update());
    }

    if (config_modified) { storage.save(); }
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    input_handler.poll(now);

    if (control.enabled()) {
        using I = djc::Control::Input;

        control.input({
            .left_x = I::fromReal(periphery.left_joystick.axis_x.read()),
            .left_y = I::fromReal(periphery.left_joystick.axis_y.read()),
            .right_x = I::fromReal(periphery.right_joystick.axis_x.read()),
            .right_y = I::fromReal(periphery.right_joystick.axis_y.read()),
        });
    }
    control.poll(now);
    ui.poll(now);
}
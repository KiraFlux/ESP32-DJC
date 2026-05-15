// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

// framework
#include <Arduino.h>

// lib
#include <kf/Logger.hpp>
#include <kf/memory/StringView.hpp>

// djc
#include "djc/ConfigManager.hpp"
#include "djc/ManualInput.hpp"
#include "djc/MavlinkTelemetryRegistry.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/Periphery.hpp"
#include "djc/input/VirtualKeyboard.hpp"
#include "djc/prelude.hpp"

// djc::transport
#include "djc/transport/TransportLink.hpp"
#include "djc/transport/TransportRegistry.hpp"

// djc::protocol
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"

// djc::service
#include "djc/service/AutoConnectService.hpp"
#include "djc/service/Control.hpp"
#include "djc/service/DisplayManager.hpp"
#include "djc/service/InputHandler.hpp"
#include "djc/service/PeerScanningService.hpp"

// djc::ui::pages
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavlinkTelemetryPage.hpp"
#include "djc/ui/pages/PeerExplorerPage.hpp"
#include "djc/ui/pages/RawProtocolPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

static constexpr auto logger{kf::Logger::create("main")};

static djc::ConfigManager config_manager{};

static auto &virtual_keyboard{djc::input::VirtualKeyboard::instance()};

static djc::Periphery periphery{
    config_manager.config().periphery,
};

static djc::transport::TransportLink transport_link{
    config_manager.config().transport_link,
};

static djc::transport::TransportRegistry transport_registry{};

static djc::protocol::ProtocolLink protocol_link{
    config_manager.config().protocol_link,
};

static djc::protocol::ProtocolRegistry protocol_registry{
    config_manager.config().protocol_registry,
};

static djc::MavlinkTelemetryRegistry mavlink_telemetry_registry{};

static djc::PeerFavoritesRegistry peer_favoriter_registry{
    {config_manager.config().peer_favorites.data(), config_manager.config().peer_favorites.size()},
};

// services

static djc::service::InputHandler input_handler{
    config_manager.config().input_handler,
    periphery.right_joystick,
    periphery.left_button_listener,
    periphery.right_button_listener,
};

static djc::service::PeerScanningService peer_scanner{
    config_manager.config().peer_scanner,
    transport_link,
};

static djc::service::AutoConnectService auto_connect_service{
    config_manager.config().auto_connect_service,
    transport_link,
};

static djc::service::Control control{
    transport_link,
    protocol_link,
};

static djc::service::DisplayManager<djc::DisplayDriver> display_manager{
    periphery.display,
    transport_link,
};

static auto &ui{djc::ui::UI::instance()};

// pages

static djc::ui::pages::RootPage root_page{};

static djc::ui::pages::PeerExplorerPage peer_explorer_page{
    root_page,
    transport_link,
    peer_scanner,
    peer_favoriter_registry,
};

static djc::ui::pages::MavlinkTelemetryPage mavlink_telemetry_page{
    root_page,
    protocol_registry,
    protocol_link,
    mavlink_telemetry_registry,
};

static djc::ui::pages::RawProtocolPage raw_protocol_page{
    root_page,
    protocol_registry,
    protocol_link,
    transport_link,
};

static djc::ui::pages::ConfigPage config_page{
    root_page,
    config_manager,
    peer_favoriter_registry,
};

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    config_manager.load();
    peer_favoriter_registry.init();
    config_page.init();

    if (not periphery.init()) {
        logger.error("Periphery init failed. Resseting periphery config to defaults");
        config_manager.config().periphery = djc::Periphery::Config::defaults();
        config_manager.modified(true);
    }

    if (not config_manager.config().periphery.joystick_axes_tuned) {
        logger.debug("Tunning axes..");
        periphery.tune(config_manager.config().periphery);
        config_manager.modified(true);
    }

    display_manager.init();

    if (not transport_registry.espnow().init()) {
        logger.error("failed to initialize espnow transport");
    }

    transport_link.transport(transport_registry.get(config_manager.config().init_transport_kind));

    transport_link.onReceive([](const djc::transport::PeerAddress &, kf::memory::Slice<const kf::u8> buffer) {
        protocol_link.receive(buffer);
    });

    protocol_registry.mavlink().callback([](const mavlink_message_t &message) {
        mavlink_telemetry_registry.update(static_cast<kf::math::Milliseconds>(millis()), message);
    });

    protocol_link.protocol(protocol_registry.get(config_manager.config().init_protocol_mode));

    peer_scanner.init();

    auto_connect_service.callback([](const djc::transport::PeerAddress &address) -> void {
        logger.info("Auto Connect");
        (void) transport_link.connect(address);
    });

    {
        using E = djc::ui::UI::Event;

        input_handler.onLeftButton([]() {
            if (virtual_keyboard.active()) {
                virtual_keyboard.quit();
            } else {
                control.enabled(not control.enabled());
                display_manager.showConnectionStatusOverlay(control.enabled());
            }

            ui.addEvent(E::update());
        });

        input_handler.onRightButton([]() {
            if (control.enabled()) { return; }

            ui.addEvent(E::widgetClick());
        });

        input_handler.onDirection([](djc::service::InputHandler::JoystickListener::Direction direction) {
            static constexpr E navigation_event_from_direction[4] = {
                E::pageCursorMove(-1),// Up
                E::pageCursorMove(+1),// Down
                E::widgetValue(-1),   // Left
                E::widgetValue(+1),   // Right
            };

            static constexpr E virtual_keyboard_event_from_direction[4] = {
                E::widgetValue(0),// Up
                E::widgetValue(1),// Down
                E::widgetValue(2),// Left
                E::widgetValue(3),// Right
            };

            if (control.enabled()) { return; }

            const auto table = virtual_keyboard.active() ? virtual_keyboard_event_from_direction : navigation_event_from_direction;
            ui.addEvent(table[static_cast<kf::u8>(direction)]);
        });

        // apply page links
        root_page.attach(peer_explorer_page);
        root_page.attach(mavlink_telemetry_page);
        root_page.attach(raw_protocol_page);
        root_page.attach(config_page);

        ui.bindPage(root_page);
        ui.addEvent(E::update());
    }

    if (config_manager.modified()) { config_manager.save(); }
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = static_cast<kf::math::Milliseconds>(millis());
    input_handler.poll(now);
    transport_link.poll(now);
    peer_scanner.poll(now);

    if (auto_connect_service.config().enabled and not auto_connect_service.target().hasValue()) {
        const auto favorites = peer_favoriter_registry.all();

        if (favorites.size() > 0) {
            auto most_trusted_favorite_index = 0u;

            for (auto index = 1u; index < favorites.size(); index += 1) {
                if (favorites[index].hasValue() and favorites[index].value().trust > favorites[most_trusted_favorite_index].value().trust) {
                    most_trusted_favorite_index = index;
                }
            }

            if (const auto &most_trusted = favorites[most_trusted_favorite_index]; most_trusted.hasValue()) {
                for (const auto &peer: peer_scanner.peers()) {
                    if (peer.hasValue() and peer.value().address == most_trusted.value().address) {
                        auto_connect_service.target(most_trusted.value().address);
                        break;
                    }
                }
            }
        }
    }
    auto_connect_service.poll(now);

    if (control.enabled()) {
        using I = djc::ManualInput;

        const I control_input{
            .left_x = I::fromNormalized(periphery.left_joystick.axis_x.read()),
            .left_y = I::fromNormalized(periphery.left_joystick.axis_y.read()),
            .right_x = I::fromNormalized(periphery.right_joystick.axis_x.read()),
            .right_y = I::fromNormalized(periphery.right_joystick.axis_y.read()),
        };

        // todo debug log

        control.input(control_input);
    }
    control.poll(now);
    ui.poll(now);
    display_manager.poll(now);
}
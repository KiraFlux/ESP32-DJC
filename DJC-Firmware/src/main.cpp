// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

// framework
#include <Arduino.h>

// lib
#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/memory/StringView.hpp>

// djc
#include "djc/AutoConnectService.hpp"
#include "djc/ConfigManager.hpp"
#include "djc/Control.hpp"
#include "djc/DisplayManager.hpp"
#include "djc/ManualInput.hpp"
#include "djc/MavlinkTelemetryRegistry.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/PeerScanner.hpp"
#include "djc/Periphery.hpp"

#include "djc/input/InputHandler.hpp"
#include "djc/input/VirtualKeyboard.hpp"

#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"

#include "djc/transport/EspNowTransport.hpp"
#include "djc/transport/TransportLink.hpp"

#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavlinkTelemetryPage.hpp"
#include "djc/ui/pages/PeerExplorerPage.hpp"
#include "djc/ui/pages/RawProtocolPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

// services

static auto &ui{djc::ui::UI::instance()};

static auto &storage{djc::ConfigManager::instance()};

static auto &virtual_keyboard{djc::input::VirtualKeyboard::instance()};

static djc::Periphery periphery{
    storage.config().periphery,
};

static djc::InputHandler input_handler{
    storage.config().input_handler,
    periphery.right_joystick,
    periphery.left_button_listener,
    periphery.right_button_listener,
};

static djc::transport::EspNowTransport espnow_transport{};

static djc::transport::TransportLink transport_link{
    storage.config().transport_link,
};

static djc::protocol::ProtocolLink protocol_link{
    storage.config().protocol_link,
};

static djc::protocol::ProtocolRegistry protocol_registry{
    storage.config().protocol_registry,
};

static djc::MavlinkTelemetryRegistry mavlink_telemetry_registry{};

static djc::PeerFavoritesRegistry peer_favoriter_registry{
    {storage.config().peer_favorites.data(), storage.config().peer_favorites.size()},
};

static djc::PeerScanner peer_scanner{
    storage.config().peer_scanner,
    transport_link,
};

static djc::AutoConnectService auto_connect_service{
    storage.config().auto_connect_service,
    transport_link,
};

static djc::Control control{
    transport_link,
    protocol_link,
};

static djc::DisplayManager display_manager{
    periphery.display,
    control,
    transport_link,
};

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
    peer_favoriter_registry,
};

void setup() {
    static constexpr auto logger{kf::Logger::create("setup")};

    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    storage.load();
    peer_favoriter_registry.init();

    if (not periphery.init()) {
        logger.error("Periphery init failed. Resseting periphery config to defaults");
        storage.config().periphery = djc::Periphery::Config::defaults();
        storage.modified(true);
    }

    if (not storage.config().periphery.joystick_axes_tuned) {
        logger.debug("Tunning axes..");
        periphery.tune(storage.config().periphery);
        storage.modified(true);
    }

    display_manager.init();

    if (espnow_transport.init()) {
        transport_link.transport(espnow_transport);
    } else {
        logger.error("failed to initialize espnow transport");
    }

    protocol_link.protocol(protocol_registry.get(storage.config().init_protocol_mode));

    transport_link.onReceive([](const djc::transport::PeerAddress &, kf::memory::Slice<const kf::u8> buffer) {
        protocol_link.receive(buffer);
    });

    protocol_registry.mavlink().callback([](const mavlink_message_t &message) {
        mavlink_telemetry_registry.update(static_cast<kf::math::Milliseconds>(millis()), message);
    });

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

    if (storage.modified()) { storage.save(); }
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

            const auto &most_trusted = favorites[most_trusted_favorite_index].value();

            for (const auto &peer: peer_scanner.peers()) {
                if (peer.hasValue() and peer.value().address == most_trusted.address) {
                    auto_connect_service.target(djc::AutoConnectService::Target{
                        .address = most_trusted.address,
                        .last_seen = now,
                    });
                    break;
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
}
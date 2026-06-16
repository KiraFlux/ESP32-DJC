// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

// framework
#include <Arduino.h>

// toolkit
#include <kf/Logger.hpp>
#include <kf/Slice.hpp>
#include <kf/memory/StringView.hpp>

// djc
#include "djc/prelude.hpp"

// djc::system
#include "djc/system/ConfigSystem.hpp"
#include "djc/system/ControlSystem.hpp"
#include "djc/system/GraphicsSystem.hpp"
#include "djc/system/InputSystem.hpp"
#include "djc/system/PeerSystem.hpp"
#include "djc/system/PeripherySystem.hpp"
#include "djc/system/ProtocolSystem.hpp"
#include "djc/system/TransportSystem.hpp"
#include "djc/system/UiSystem.hpp"

// djc::ui
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavlinkTelemetryPage.hpp"
#include "djc/ui/pages/PeerExplorerPage.hpp"
#include "djc/ui/pages/RawProtocolPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

static constexpr auto loop_rate_hz{50};

static constexpr auto logger{kf::Logger::create("main")};

// systems

static djc::system::ConfigSystem config_system{};

static auto &config{config_system.service().config()};

static djc::system::PeripherySystem periphery_system{config};

static djc::system::TransportSystem transport_system{config};

static djc::system::ProtocolSystem protocol_system{config};

static djc::system::ControlSystem control_system{
    periphery_system.periphery(),
    transport_system.link(),
    protocol_system.link(),
};

static djc::system::PeerSystem peer_system{
    config,
    transport_system.link(),
};

static djc::system::InputSystem input_system{
    config,
    periphery_system.periphery(),
};

static djc::system::UiSystem ui_system{config};

static djc::system::GraphicsSystem<djc::DisplayDriver> graphics_system{
    periphery_system.periphery().display,
    ui_system.virtualKeyboard(),
};

// ui pages

static djc::ui::pages::PeerExplorerPage peer_explorer_page{
    ui_system.service(),
    ui_system.rootPage(),
    transport_system.link(),
    peer_system.scanningService(),
    peer_system.favoritesRegistry(),
};

static djc::ui::pages::MavlinkTelemetryPage mavlink_telemetry_page{
    ui_system.service(),
    ui_system.rootPage(),
    protocol_system.protocolRegistry(),
    protocol_system.link(),
    protocol_system.mavlinkTelemetryRegistry(),
};

static djc::ui::pages::RawProtocolPage raw_protocol_page{
    ui_system.service(),
    ui_system.rootPage(),
    protocol_system.protocolRegistry(),
    protocol_system.link(),
    transport_system.link(),
};

static djc::ui::pages::ConfigPage config_page{
    ui_system.service(),
    ui_system.rootPage(),
    config_system.service(),
    peer_system.favoritesRegistry(),
};

// navigation to event maps

using UiEvent = djc::ui::UI::Traits::EventImpl;

static constexpr UiEvent navigation_event_from_direction[4]{
    UiEvent::pageCursorMove(-1),// Up
    UiEvent::pageCursorMove(+1),// Down
    UiEvent::widgetValue(-1),   // Left
    UiEvent::widgetValue(+1),   // Right
};

static constexpr UiEvent virtual_keyboard_event_from_direction[4]{
    UiEvent::widgetValue(0),// Up
    UiEvent::widgetValue(1),// Down
    UiEvent::widgetValue(2),// Left
    UiEvent::widgetValue(3),// Right
};

// callbacks

static void onReceiveFromPeer(const djc::transport::PeerAddress &address, kf::Slice<const kf::u8> buffer) noexcept {
    (void) address;

    protocol_system.link().receive(buffer);
}

static void onReceiveFromLogger(kf::memory::StringView str) noexcept {
    Serial.write(str.data(), str.size());
}

static void onPrimaryButtonClick() noexcept {
    if (control_system.service().enabled()) { return; }

    ui_system.service().addEvent(UiEvent::widgetClick());
}

static void onSecondaryButtonClick() noexcept {
    if (ui_system.virtualKeyboard().active()) {
        ui_system.virtualKeyboard().quit();
    } else {
        control_system.service().enabled(not control_system.service().enabled());
    }

    ui_system.service().addEvent(UiEvent::update());
}

static void onPrimaryJoystickDirection(djc::service::InputHandler::JoystickListener::Direction direction) noexcept {
    if (control_system.service().enabled()) { return; }

    const auto table = ui_system.virtualKeyboard().active() ? virtual_keyboard_event_from_direction : navigation_event_from_direction;
    ui_system.service().addEvent(table[static_cast<kf::u8>(direction)]);
}

static void onUiRendered(kf::memory::StringView str) {
    using Palette = std::decay_t<decltype(graphics_system.service())>::Palette;

    if (control_system.service().enabled()) {
        if (transport_system.link().connected()) {
            graphics_system.service().overlay(transport_system.link().activePeerAddress().unwrap().toString().view(), Palette::light_green);
        } else {
            graphics_system.service().overlay("Disconnected", Palette::light_yellow);
        }
    } else {
        if (const auto &p = ui_system.service().activePage(); p.isSome()) {
            if (const auto &widget = p.unwrap().selectedWidget(); widget.isSome()) {
                graphics_system.service().overlay(widget.unwrap().hint(), Palette::light_gray);
            }
        }
    }

    graphics_system.service().onRender(str);
}

// setups

static bool setupPeriphery(djc::Config &config) noexcept {
    if (not config.periphery.joystick_axes_tuned) {
        logger.debug("Tunning axes..");
        periphery_system.periphery().tune(config.periphery);
        return true;
    }

    return false;
}

static bool setupGraphics(djc::Config &config) noexcept {
    if (const auto &canvas = graphics_system.service().canvas(); canvas.isSome()) {
        config.render_system.text.row_max_length = canvas.unwrap().widthInGlyphs();
        config.render_system.text.rows_total = canvas.unwrap().heightInGlyphs() - 1;
        return true;
    }

    return false;
}

#define DJC_SYSTEM_INIT(__system_instance__, ...) \
    __system_instance__.init(__VA_ARGS__);        \
    logger.info("done: '" #__system_instance__ "'");

#define DJC_DO_SETUP(__setup_function__) \
    if (__setup_function__) { config_system.service().requestSave(); }

void setup() {
    Serial.begin(115200);
    kf::Logger::writer = onReceiveFromLogger;

    // init

    DJC_SYSTEM_INIT(config_system);
    DJC_SYSTEM_INIT(periphery_system);
    DJC_SYSTEM_INIT(transport_system, config.init_transport_kind);
    DJC_SYSTEM_INIT(protocol_system, config.init_protocol_mode);
    DJC_SYSTEM_INIT(peer_system, transport_system.link());
    DJC_SYSTEM_INIT(input_system);
    DJC_SYSTEM_INIT(graphics_system);
    DJC_SYSTEM_INIT(control_system);
    DJC_SYSTEM_INIT(ui_system, {&peer_explorer_page, &mavlink_telemetry_page, &raw_protocol_page, &config_page});

    // orcestre

    transport_system.link().onReceive(onReceiveFromPeer);
    peer_system.favoritesRegistry().entries({config.peer_favorites.data(), config.peer_favorites.size()});

    input_system.service().onLeftButton(onSecondaryButtonClick);
    input_system.service().onRightButton(onPrimaryButtonClick);
    input_system.service().onDirection(onPrimaryJoystickDirection);

    ui_system.renderer().callback(onUiRendered);

    DJC_DO_SETUP(setupPeriphery(config));
    DJC_DO_SETUP(setupGraphics(config));
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / loop_rate_hz};

    const auto now = static_cast<kf::math::Milliseconds>(millis());

    config_system.poll(now);
    periphery_system.poll(now);
    transport_system.poll(now);
    protocol_system.poll(now);
    peer_system.poll(now);
    input_system.poll(now);
    control_system.poll(now);
    ui_system.poll(now);
    graphics_system.poll(now);

    delay(loop_period);
}
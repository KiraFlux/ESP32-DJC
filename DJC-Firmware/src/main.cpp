// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Device.hpp"
#include "djc/ui/MainPage.hpp"
#include "djc/ui/MavLinkControlPage.hpp"
#include "djc/ui/UI.hpp"

static auto &ui = djc::UI::instance();

static auto &device = djc::Device::instance();

// pages
static djc::MavLinkControlPage mav_link_control{};

void setup() {
    // Logging setup
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    // Device setup
    device.setupPeriphery();
    device.setupGraphics();
    device.setupRender(ui.renderConfig());

    // UI setup
    auto &main_page = djc::MainPage::instance();
    main_page.widget_layout[0] = &mav_link_control.link();

    ui.bindPage(main_page);
    ui.addEvent(djc::UI::Event::update());
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    device.poll(now);
    ui.poll(now);
}
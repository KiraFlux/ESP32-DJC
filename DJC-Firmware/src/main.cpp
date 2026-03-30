// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Device.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PageManager.hpp"

static auto &ui = djc::ui::UI::instance();

static auto &device = djc::Device::instance();

// pages

void setup() {
    // Logging setup
    Serial.begin(115200);
    kf::Logger::writer = [](kf::memory::StringView str) { Serial.write(str.data(), str.size()); };

    // Device setup
    device.setupPeriphery();
    device.setupGraphics();
    device.setupRender(ui.renderConfig());

    // UI setup
    djc::ui::pages::PageManager::instance().init();
}

void loop() {
    constexpr kf::math::Milliseconds loop_period{1000 / 50};// 50 Hz
    delay(loop_period);

    const auto now = millis();
    device.poll(now);
    ui.poll(now);
}
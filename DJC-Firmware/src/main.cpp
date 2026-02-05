#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Device.hpp"
#include "djc/UI.hpp"
#include "djc/ui/MavLinkControlPage.hpp"
#include "djc/ui/TestPage.hpp"
#include "djc/ui/MainPage.hpp"


static auto &ui = djc::UI::instance();

static auto &device = djc::Device::instance();

// Unused static pages (created for static initialization)
kf_maybe_unused static djc::MavLinkControlPage mav_link_control{};

kf_maybe_unused static djc::TestPage test_page{"Test (Super-Duper-Mega long name btw)"};

void setup() {
    // Logging setup
    Serial.begin(115200);
    kf::Logger::writer = [](kf::StringView str) { Serial.write(str.data(), str.size()); };

    // Device setup
    device.setupPeriphery();
    device.setupGraphics();
    device.setupRender(ui.renderConfig());

    // UI setup
    ui.bindPage(djc::MainPage::instance());
    ui.addEvent(djc::UI::Event::update());
}

void loop() {
    constexpr kf::Milliseconds loop_period{1000 / 50}; // 50 Hz
    delay(loop_period);

    const auto now = millis();
    device.poll(now);
    ui.poll(now);
}
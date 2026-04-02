// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>
#include <MAVLink.h>
#include <cstring>

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Device.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief MAVLink protocol control page for drone/vehicle control
struct MavLinkControlPage : UI::Page {
    explicit MavLinkControlPage(UI::Page &root) noexcept :
        Page{"MAV Link Control"},
        widget_layout{{
            &root.link(),
            &log_display,
        }} {
        widgets({widget_layout.data(), widget_layout.size()});

        const auto now = millis();
        debug_timer.start(now);
        heartbeat_timer.start(now);
    }

    void onEntry() noexcept override {
        EspNow::instance().onReceiveFromUnknown(
            [this](const EspNow::Mac &, kf::memory::Slice<const kf::u8> buffer) {
                mavlink_message_t message;
                mavlink_status_t status;

                for (auto b: buffer) {
                    if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status)) {
                        onMavLinkMessage(&message);
                    }
                }
            });
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        sendManualControl(now);

        if (heartbeat_timer.expired(now)) {
            sendHeartBeat();
            heartbeat_timer.start(now);
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("MavLink")};

    kf::math::Timer debug_timer{static_cast<kf::math::Milliseconds>(100)};
    kf::math::Timer heartbeat_timer{static_cast<kf::math::Milliseconds>(2000)};

    kf::memory::ArrayString<256> log_buffer{
        ""// Extended ASCII characters for display testing
        "\xF0#0#\xF1#1#\xF2#2#\xF3#3#\xF4#4#\xF5#5#\xF6#6#\xF7#7#\n"
        "\xF8#8#\xF9#9#\xFA#A#\xFB#B#\xFC#C#\xFD#D#\xFE#E#\xFF#F#\n"
        "\xB0 0 \xB1 1 \xB2 2 \xB3 3 \xB4 4 \xB5 5 \xB6 6 \xB7 7 \n"
        "\xB8 8 \xB9 9 \xBB A \xBB B \xBC C \xBD D \xBE E \xBF F "};

    UI::Display<kf::memory::StringView> log_display{log_buffer.view()};
    kf::memory::Array<UI::Widget *, 2> widget_layout;


};

}// namespace djc::pages
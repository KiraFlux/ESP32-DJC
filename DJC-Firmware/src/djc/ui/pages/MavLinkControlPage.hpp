// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <MAVLink.h>

#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

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
    }

    void onEntry() noexcept override {
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
    }

private:
    kf::memory::ArrayString<256> log_buffer{
        ""// Extended ASCII characters for display testing
        "\xF0#0#\xF1#1#\xF2#2#\xF3#3#\xF4#4#\xF5#5#\xF6#6#\xF7#7#\n"
        "\xF8#8#\xF9#9#\xFA#A#\xFB#B#\xFC#C#\xFD#D#\xFE#E#\xFF#F#\n"
        "\xB0 0 \xB1 1 \xB2 2 \xB3 3 \xB4 4 \xB5 5 \xB6 6 \xB7 7 \n"
        "\xB8 8 \xB9 9 \xBB A \xBB B \xBC C \xBD D \xBE E \xBF F "};

    UI::Display<kf::memory::StringView> log_display{log_buffer.view()};
    kf::memory::Array<UI::Widget *, 2> widget_layout;
};

}// namespace djc::ui::pages

/*


    void onMavLinkMessage(mavlink_message_t *message) noexcept {

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                constexpr auto len{sizeof(serial_control.data)};

                serial_control.data[len - 1] = '\0';

                // callback?
                // todo
                return;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                // callback?
                // todo
                return;
            }

            default:
                // Unhandled message type
                return;
        }
*/
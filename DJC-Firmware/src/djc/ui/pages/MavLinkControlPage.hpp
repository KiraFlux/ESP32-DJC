// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <MAVLink.h>

#include <kf/Logger.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Control.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief MAVLink protocol control page for drone/vehicle control
struct MavLinkControlPage : UI::Page {
    explicit MavLinkControlPage(UI::Page &root, Control &control) noexcept :
        Page{"MAV Link Control"}, _control{control},
        widget_layout{{
            &root.link(),
            &imu_display_labeled,
            &log_display,
        }} {
        widgets({widget_layout.data(), widget_layout.size()});
    }

    void onEntry() noexcept override {
        logger.debug("entry");

        _control.mode(Control::Mode::MavLink);
        _control.onMavlinkMessage([this](mavlink_message_t *message) {
            const bool need_update = onMavLinkMessage(message);
            if (need_update) {
                UI::instance().addEvent(UI::Event::update());
            }
        });
    }

    void onExit() noexcept override {
        logger.debug("exit");

        _control.onMavlinkMessage(Control::MavLinkMessageCallback{nullptr});
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {}

private:
    static constexpr auto logger{kf::Logger::create("MavLinkControlPage")};

    Control &_control;
    kf::memory::ArrayString<256> log_buffer{
        ""// Extended ASCII characters for display testing
        "\xF0#0#\xF1#1#\xF2#2#\xF3#3#\xF4#4#\xF5#5#\xF6#6#\xF7#7#\n"
        "\xF8#8#\xF9#9#\xFA#A#\xFB#B#\xFC#C#\xFD#D#\xFE#E#\xFF#F#\n"
        "\xB0 0 \xB1 1 \xB2 2 \xB3 3 \xB4 4 \xB5 5 \xB6 6 \xB7 7 \n"
        "\xB8 8 \xB9 9 \xBB A \xBB B \xBC C \xBD D \xBE E \xBF F "};

    kf::memory::ArrayString<64> imu_display_buffer{"..."};

    UI::Display<kf::memory::StringView> log_display{log_buffer.view()};
    UI::Display<kf::memory::StringView> imu_display{imu_display_buffer.view()};
    UI::Labeled imu_display_labeled{"IMU", imu_display};

    kf::memory::Array<UI::Widget *, 3> widget_layout;

    [[nodiscard]] bool onMavLinkMessage(mavlink_message_t *message) noexcept {
        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                constexpr auto len{sizeof(serial_control.data)};
                serial_control.data[len - 1] = '\0';

                std::copy(serial_control.data, serial_control.data + serial_control.count, log_buffer.data());
                log_display.value(log_buffer.view());

                return true;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) imu_display_buffer.format(
                    "%+.3f %+.3f %+.3f",
                    imu.xacc * 1000,
                    imu.yacc * 1000,
                    imu.zacc * 1000);
                imu_display.value(imu_display_buffer.view());

                return true;
            }

            default:
                // Unhandled message type
                return false;
        }
    }
};

}// namespace djc::ui::pages
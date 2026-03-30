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

    void onMavLinkMessage(mavlink_message_t *message) {
        logger.debug(kf::memory::ArrayString<32>::formatted("MAVLink message ID: %d", message->msgid).view());

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(log_buffer.data(), serial_control.data, serial_control.count);
                log_buffer[kf::min(kf::usize(serial_control.count), log_buffer.size() - 1)] = '\0';
                log_display.value(log_buffer.view());
                break;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) log_buffer.format(
                    "Accel: %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f);
                log_display.value(log_buffer.view());
                break;
            }

            default:
                // Unhandled message type
                break;
        }
    }

    static void sendHeartBeat() {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127,            // System ID
            MAV_COMP_ID_OSD,// Component ID
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0// Base mode, Custom mode, system status
        );

        sendMavlinkToEspnow(message);
    }

    void sendManualControl(kf::math::Milliseconds now) {
        constexpr auto scale = 1000;

        const auto &controller_values = Device::instance().controllerValues();
        const auto left_x = controller_values.left_x * scale;
        const auto left_y = controller_values.left_y * scale;
        const auto right_x = controller_values.right_x * scale;
        const auto right_y = controller_values.right_y * scale;

        if (debug_timer.expired(now)) {
            logger.debug(kf::memory::ArrayString<64>::formatted(
                             "L: (%.3f, %.3f) R: (%.3f, %.3f)",
                             left_x, left_y, right_x, right_y)
                             .view());
            debug_timer.start(now);
        }

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x: pitch (right Y)
            static_cast<kf::i16>(right_y),
            // y: roll (right X)
            static_cast<kf::i16>(right_x),
            // z: thrust (left Y)
            static_cast<kf::i16>(left_y),
            // r: yaw (left X)
            static_cast<kf::i16>(left_x),
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        auto &peer_opt = Device::instance().espnowPeer();
        if (peer_opt.hasValue()) {
            (void) peer_opt.value().writeBuffer(kf::memory::Slice<const kf::u8>{buffer, len});
        }
    }
};

}// namespace djc::pages
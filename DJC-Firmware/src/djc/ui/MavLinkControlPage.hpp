#pragma once

#pragma once

#include <Arduino.h>
#include <MAVLink.h>
#include <cstring>

#include <kf/Logger.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/math/time/Timer.hpp>

#include "djc/UI.hpp"


namespace djc {

struct MavLinkControlPage : UI::Page {

private:

    kf::Timer heartbeat_timer{static_cast<kf::Milliseconds>(2000)};
    kf::ArrayString<64> text_buffer{"Waiting for message"};
    kf::StringView text_view{text_buffer.view()};

    UI::Display <kf::StringView> text_display{*this, text_view};

public:

    explicit MavLinkControlPage() :
        Page{"MAV Link Control"} {}

    void onEntry() override {
        kf::EspNow::instance().setUnknownReceiveHandler([this](const kf::EspNow::Mac &, kf::Slice<const kf::u8> buffer) {
            mavlink_message_t message;
            mavlink_status_t status;

            for (int i = 0; i < buffer.size(); i += 1) {
                if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status)) {
                    onMavLinkMessage(&message);
                }
            }
        });
    }

    void onUpdate() override {
        sendManualControl();

        if (heartbeat_timer.ready(millis())) {
            sendHeartBeat();
        }
    }

private:

    void onMavLinkMessage(mavlink_message_t *message) {
        kf_Logger_debug("%d", message->msgid);

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;

                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(text_buffer.data(), serial_control.data, serial_control.count);
                text_buffer[kf::min(kf::usize(serial_control.count), text_buffer.size() - 1)] = '\0';
            }
                return;

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;

                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) text_buffer.format(
                    "A %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f
                );

            }

            default: //
                return;
        }
    }

//    static void sendSerialControl() {
//        mavlink_message_t message;
//        // mavlink_msg_serial_control_encode(
//        //     127,
//
//        // );
//        sendMavlinkToEspnow(message);
//    }

    static void sendHeartBeat() {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127,
            MAV_COMP_ID_OSD,
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            //
            0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendManualControl() {
        constexpr auto scale = 1000;

        auto &periphery = Periphery::instance();
        const auto left_x = periphery.left_joystick.axis_x.read() * scale;
        const auto left_y = periphery.left_joystick.axis_y.read() * scale;
        const auto right_x = periphery.right_joystick.axis_x.read() * scale;
        const auto right_y = periphery.right_joystick.axis_y.read() * scale;

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x : pitch
            static_cast<kf::i16>(right_y),
            // y : roll
            static_cast<kf::i16>(right_x),
            // z : thrust
            static_cast<kf::i16>(left_y),
            // r : yaw
            static_cast<kf::i16>(left_x),
            //
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);
        (void) Periphery::instance().espnow_peer.value().sendBuffer(kf::Slice<const kf::u8>{buffer, len});
    }

};

}
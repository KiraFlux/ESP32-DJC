#pragma once

#include <cstring>
#include <MAVLink.h>

#include <kf/sys.hpp>
#include <kf/aliases.hpp>
#include <kf/String.hpp>
#include <kf/tools/meta/Singleton.hpp>
#include <kf/tools/time/Timer.hpp>

#include "djc/Periphery.hpp"


namespace djc {

struct MavLinkControl : kf::sys::Behavior, kf::tools::Singleton<MavLinkControl> {
    friend struct Singleton<MavLinkControl>;

    kf::sys::JoystickComponent left_joystick{};
    kf::sys::JoystickComponent right_joystick{};

    std::array<char, 70> text_buffer{"MAV Link"};
    kf::sys::TextComponent text_box{text_buffer.data()};
    kf::tools::Timer heartbeat_timer{static_cast<kf::Milliseconds>(2000)};

    void updateLayout(kf::gfx::Canvas &root) override {
        auto [up, down] = root.splitVertically<2>({4, 4});

        const auto [left_joy, right_joy] = up.splitHorizontally<2>({});
        left_joystick.canvas = left_joy;
        right_joystick.canvas = right_joy;

        text_box.canvas = down;
        text_box.canvas.auto_next_line = true;
    }

    void update() override {
        auto &periphery = Periphery::instance();

        sendManualControl();

        if (heartbeat_timer.ready()) {
            sendHeartBeat();
        }

        periphery.right_button.poll();
    }

    void onEntry() override {
        kf::EspNow::instance().setUnknownReceiveHandler([this](const kf::EspNow::Mac &, const kf::slice<const void> &buffer) {
            mavlink_message_t message;
            mavlink_status_t status;

            for (int i = 0; i < buffer.size(); i += 1) {
                if (mavlink_parse_char(MAVLINK_COMM_0, static_cast<const kf::u8 *>(buffer.data())[i], &message, &status)) {
                    onMavLinkMessage(&message);
                }
            }
        });

        auto &periphery = Periphery::instance();
        periphery.right_button.handler = []() {
            // todo serialControlSend("help")
        };
    }

private:
    void onMavLinkMessage(mavlink_message_t *message) {
        kf_Logger_debug("%d", message->msgid);

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;

                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(text_buffer.data(), serial_control.data, serial_control.count);
            }
                return;

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;

                mavlink_msg_scaled_imu_decode(message, &imu);

                kf::formatTo(
                    text_buffer,
                    "A %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f
                );
            }

            default:return;
        }
    }

    static void sendSerialControl() {
        mavlink_message_t message;
        // mavlink_msg_serial_control_encode(
        //     127,

        // );
        sendMavlinkToEspnow(message);
    }

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

    void sendManualControl() {
        constexpr auto scale = 1000;

        auto &periphery = Periphery::instance();
        left_joystick.x = periphery.left_joystick.axis_x.read();
        left_joystick.y = periphery.left_joystick.axis_y.read();
        right_joystick.x = periphery.right_joystick.axis_x.read();
        right_joystick.y = periphery.right_joystick.axis_y.read();

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x : pitch
            static_cast<kf::i16>(scale * right_joystick.y),
            // y : roll
            static_cast<kf::i16>(scale * right_joystick.x),
            // z : thrust
            static_cast<kf::i16>(scale * left_joystick.y),
            // r : yaw
            static_cast<kf::i16>(scale * left_joystick.x),
            //
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];

        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        (void) Periphery::instance().espnow_peer.value().sendBuffer(
            kf::slice<const void>{
                .ptr = static_cast<const void *>(buffer),
                .size = len
            });
    }

    MavLinkControl() {
        addComponent(left_joystick);
        addComponent(right_joystick);
        addComponent(text_box);
    }
};

}// namespace djc

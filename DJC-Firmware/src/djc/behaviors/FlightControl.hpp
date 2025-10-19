#pragma once

#include <kf/sys.hpp>
#include <MAVLink.h>

#include "djc/Periphery.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct FlightControl : kf::sys::Behavior, Singleton<FlightControl> {
    friend struct Singleton<FlightControl>;

    kf::sys::JoystickComponent left_joystick;
    kf::sys::JoystickComponent right_joystick;
    kf::sys::FlagComponent mavlink_mode_enable{"MAV Link"};

    struct SimpleControlPacket {
        rs::f32 left_x;
        rs::f32 left_y;
        rs::f32 right_x;
        rs::f32 right_y;
    };

    void updateLayout(kf::gfx::Canvas &root) override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        mavlink_mode_enable.canvas = up;

        const auto [left_joy, right_joy] = down.splitHorizontally<2>({});
        left_joystick.canvas = left_joy;
        right_joystick.canvas = right_joy;
    }

    void update() override {
        auto &periphery = djc::Periphery::instance();

        left_joystick.x = periphery.left_joystick.axis_x.read();
        left_joystick.y = periphery.left_joystick.axis_y.read();
        right_joystick.x = periphery.right_joystick.axis_x.read();
        right_joystick.y = periphery.right_joystick.axis_y.read();
        periphery.left_button.poll();

        if (bool(mavlink_mode_enable)) {
            mavlink_message_t message;
            rs::u8 buffer[MAVLINK_MAX_PACKET_LEN];

            constexpr auto scale = 1000;

            mavlink_msg_manual_control_pack(
                127, MAV_COMP_ID_PARACHUTE, &message, 1,
                // x : pitch
                static_cast<rs::i16>(scale * right_joystick.y),
                // y : roll
                static_cast<rs::i16>(scale * right_joystick.x),
                // z : thrust
                static_cast<rs::i16>(scale * left_joystick.y),
                // r : yaw
                static_cast<rs::i16>(scale * left_joystick.x),
                //
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            const auto len = mavlink_msg_to_send_buffer(buffer, &message);

            (void) periphery.espnow_node.send(static_cast<const void *>(buffer), len);
        } else {
            (void) periphery.espnow_node.send(SimpleControlPacket{
                .left_x=left_joystick.x,
                .left_y=left_joystick.y,
                .right_x=right_joystick.x,
                .right_y=right_joystick.y,
            });
        }
    }

    void onEntry() override {
        djc::Periphery::instance().left_button.handler = [this]() { mavlink_mode_enable.toggle(); };
    }

private:
    FlightControl() {
        addComponent(left_joystick);
        addComponent(right_joystick);
        addComponent(mavlink_mode_enable);
    }
};

}// namespace djc

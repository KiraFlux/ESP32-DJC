#pragma once

#include <kf/sys.hpp>
#include <MAVLink.h>

#include "djc/Periphery.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct FlightBehavior : kf::sys::Behavior, Singleton<FlightBehavior> {
    friend struct Singleton<FlightBehavior>;

    kf::sys::JoystickElement left_joystick_element;
    kf::sys::JoystickElement right_joystick_element;
    kf::sys::FlagElement mavlink_mode_flag;

    bool mav_link_mode{true};

    struct {
        float left_x{0}, left_y{0};
        float right_x{0}, right_y{0};
    } packet{};

    void bindPainters(kf::Painter &root) override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        auto [left_joy, right_joy] = down.splitHorizontally<2>({});

        left_joystick_element.painter = left_joy;
        right_joystick_element.painter = right_joy;
        mavlink_mode_flag.painter = up;
    }

    void loop() override {
        auto &periphery = djc::Periphery::instance();
        packet.left_x = periphery.left_joystick.axis_x.read();
        packet.left_y = periphery.left_joystick.axis_y.read();
        packet.right_x = periphery.right_joystick.axis_x.read();
        packet.right_y = periphery.right_joystick.axis_y.read();

        if (mav_link_mode) {

            mavlink_message_t msg;
            uint8_t buf[MAVLINK_MAX_PACKET_LEN];

            constexpr auto scale = 1000;

            mavlink_msg_manual_control_pack(
                127, MAV_COMP_ID_PARACHUTE, &msg, 1,
                // x : pitch
                static_cast<int16_t>(scale * packet.right_y),
                // y : roll
                static_cast<int16_t>(scale * packet.right_x),
                // z : thrust
                static_cast<int16_t>(scale * packet.left_y),
                // r : yaw
                static_cast<int16_t>(scale * packet.left_x),
                //
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            const auto len = mavlink_msg_to_send_buffer(buf, &msg);

            (void)periphery.espnow_node.send(static_cast<const void *>(buf), len);

        } else {
            (void)periphery.espnow_node.send(packet);
        }

        periphery.left_button.poll();
    }

    void onBind() override {
        auto &periphery = djc::Periphery::instance();

        periphery.left_button.handler = [this]() {
            mav_link_mode = not mav_link_mode;
        };

        left_joystick_element.bindAxis(packet.left_x, packet.left_y);
        right_joystick_element.bindAxis(packet.right_x, packet.right_y);
        mavlink_mode_flag.flag = &mav_link_mode;
        mavlink_mode_flag.label = "MavLink Mode";
    }

private:
    FlightBehavior() {
        add(left_joystick_element);
        add(right_joystick_element);
        add(mavlink_mode_flag);
    }
};

}// namespace djc

#pragma once

#include <MAVLink.h>
#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"

namespace djc {

struct MavLinkControl : kf::sys::Behavior, kf::tools::Singleton<MavLinkControl> {
    friend struct Singleton<MavLinkControl>;

    kf::sys::JoystickComponent left_joystick{};
    kf::sys::JoystickComponent right_joystick{};

    kf::sys::TextComponent text_box{"MAV Link"};

    void updateLayout(kf::gfx::Canvas &root) override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        text_box.canvas = up;

        const auto [left_joy, right_joy] = down.splitHorizontally<2>({});
        left_joystick.canvas = left_joy;
        right_joystick.canvas = right_joy;
    }

    void update() override {
        constexpr auto scale = 1000;

        auto &periphery = Periphery::instance();

        mavlink_message_t message;
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];

        left_joystick.x = periphery.left_joystick.axis_x.read();
        left_joystick.y = periphery.left_joystick.axis_y.read();
        right_joystick.x = periphery.right_joystick.axis_x.read();
        right_joystick.y = periphery.right_joystick.axis_y.read();

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

        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        (void) periphery.espnow_node.send(static_cast<const void *>(buffer), len);
    }

    void onEntry() override {}

private:
    MavLinkControl() {
        addComponent(left_joystick);
        addComponent(right_joystick);
        addComponent(text_box);
    }
};

}// namespace djc

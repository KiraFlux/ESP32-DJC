#pragma once

#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"

namespace djc {

struct SimpleControl : kf::sys::Behavior, kf::tools::Singleton<SimpleControl> {
    friend struct Singleton<SimpleControl>;

    kf::sys::JoystickComponent left_joystick{};
    kf::sys::JoystickComponent right_joystick{};
    kf::sys::TextComponent text_box{"Simple Control"};

    struct SimpleControlPacket {
        kf::f32 left_x;
        kf::f32 left_y;
        kf::f32 right_x;
        kf::f32 right_y;
    };

    void updateLayout(kf::gfx::Canvas &root) override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        text_box.canvas = up;

        const auto [left_joy, right_joy] = down.splitHorizontally<2>({});
        left_joystick.canvas = left_joy;
        right_joystick.canvas = right_joy;
    }

    void update() override {
        auto &periphery = Periphery::instance();

        left_joystick.x = periphery.left_joystick.axis_x.read();
        left_joystick.y = periphery.left_joystick.axis_y.read();
        right_joystick.x = periphery.right_joystick.axis_x.read();
        right_joystick.y = periphery.right_joystick.axis_y.read();

        (void) periphery.espnow_peer.value().sendPacket(SimpleControlPacket{
            .left_x = left_joystick.x,
            .left_y = left_joystick.y,
            .right_x = right_joystick.x,
            .right_y = right_joystick.y,
        });
    }

private:
    SimpleControl() {
        addComponent(left_joystick);
        addComponent(right_joystick);
        addComponent(text_box);
    }
};

}// namespace djc

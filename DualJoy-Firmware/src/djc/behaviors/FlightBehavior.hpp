#pragma once

#include <KiraFlux-GUI.hpp>

#include "djc/Periphery.hpp"
#include "djc/gui/FlagDisplay.hpp"
#include "djc/gui/JoyWidget.hpp"
#include "djc/tools/Singleton.hpp"

namespace djc {

struct FlightBehavior : kfgui::Behavior, Singleton<FlightBehavior> {
    friend struct Singleton<FlightBehavior>;

    JoyWidget left_joy_widget;
    JoyWidget right_joy_widget;
    FlagDisplay toggle_mode;

    struct {
        float left_x{0}, left_y{0};
        float right_x{0}, right_y{0};
        bool toggle_mode{false};
    } packet{};

    void bindPainters(kf::Painter &root) noexcept override {
        auto [up, down] = root.splitVertically<2>({1, 7});
        auto [left_joy, right_joy] = down.splitHorizontally<2>({});

        left_joy_widget.painter = left_joy;
        right_joy_widget.painter = right_joy;
        toggle_mode.painter = up;
    }

    void loop() noexcept override {
        auto &periphery = djc::Periphery::instance();

        packet.left_x = periphery.left_joystick.axis_x.read();
        packet.left_y = periphery.left_joystick.axis_y.read();
        packet.right_x = periphery.right_joystick.axis_x.read();
        packet.right_y = periphery.right_joystick.axis_y.read();

        periphery.left_button.poll();
        periphery.espnow_node.send(packet);
    }

    void onBind() noexcept override {
        auto &periphery = djc::Periphery::instance();

        periphery.left_button.handler = [this]() {
            packet.toggle_mode = not packet.toggle_mode;
        };

        left_joy_widget.bindAxis(packet.left_x, packet.left_y);
        right_joy_widget.bindAxis(packet.right_x, packet.right_y);
        toggle_mode.flag = &packet.toggle_mode;
        toggle_mode.label = "mode";
    }

private:
    FlightBehavior() {
        add(left_joy_widget);
        add(right_joy_widget);
        add(toggle_mode);
    }
};

}// namespace djc

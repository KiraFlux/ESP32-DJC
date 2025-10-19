#pragma once

#include <kf/Button.hpp>
#include <kf/Joystick.hpp>
#include <kf/JoystickListener.hpp>
#include <kf/Logger.hpp>
#include <kf/SSD1306.h>

#include "djc/remote/EspnowNode.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct Periphery : Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    kf::Button left_button{GPIO_NUM_15, kf::Button::Mode::PullUp};

    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};

    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};

    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};

    kf::JoystickListener left_joystick_listener{left_joystick};

    EspnowNode espnow_node{{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    kf::SSD1306 display_driver{};

    [[nodiscard]] bool init() {
        kf_Logger_info("init");

        display_driver.init();
        Wire.setClock(1000000u);
        display_driver.update();

        left_joystick.init();
        right_joystick.init();
        left_button.init(false);
        right_button.init(false);

        if (not espnow_node.init()) { return false; }

        kf_Logger_info("OK");
        return true;
    }

    void configure() {
        left_joystick.axis_x.inverted = true;
        right_joystick.axis_y.inverted = true;
    }

    void calibrate(int joystick_samples = 500) {
        left_joystick.calibrate(joystick_samples);
        right_joystick.calibrate(joystick_samples);
    }

};

}// namespace djc

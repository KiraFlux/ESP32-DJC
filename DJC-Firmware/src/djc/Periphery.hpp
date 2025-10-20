#pragma once

#include <kf/Button.hpp>
#include <kf/Joystick.hpp>
#include <kf/JoystickListener.hpp>
#include "kf/Logger.hpp"
#include <kf/SSD1306.h>
#include "kf/tools/meta/Singleton.hpp"

#include "djc/remote/EspnowNode.hpp"


namespace djc {

struct Periphery : kf::tools::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    kf::Button left_button{GPIO_NUM_15, kf::Button::Mode::PullUp};

    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};

    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};

    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};

    kf::JoystickListener left_joystick_listener{left_joystick};

    EspnowNode espnow_node{{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    kf::SSD1306 screen_driver{};

    [[nodiscard]] bool init() {
        kf_Logger_info("init");

        if (not screen_driver.init()) {
            kf_Logger_error("Screen driver error");
            return false;
        }

        Wire.setClock(1000000u);
        screen_driver.flush();

        left_joystick.init();
        right_joystick.init();
        left_button.init(kf::Button::PullType::Internal);
        right_button.init(kf::Button::PullType::Internal);

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

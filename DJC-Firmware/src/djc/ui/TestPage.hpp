#pragma once

#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/math/time/Timer.hpp>

#include "djc/UI.hpp"


namespace djc {

struct TestPage : UI::Page {
    float value{12.3456};

    using Boiler = UI::Labeled <UI::ComboBox<int, 3>>;

    Boiler::Impl::Value item{0};
    Boiler boiler{
        *this,
        "Boiler",
        Boiler::Impl{
            item,
            {
                {
                    {"ice", 1},
                    {"water", 20},
                    {"steam", 300},
                }
            }
        }
    };

    UI::Display<int> display_2{
        *this,
        item
    };

    UI::SpinBox<float> spin_box{
        *this,
        value,
        0.1f,
        UI::SpinBox<float>::Mode::Arithmetic
    };

    UI::Button button{
        *this,
        "button",
        [this]() {
            kf_Logger_debug("button click");
            value = -value * 1.4f;
        }
    };

    UI::CheckBox check_box{
        *this,
        [](bool state) {
            kf_Logger_debug("state: %s", state ? "ON" : "OFF");
        }
    };

    UI::Display<float> display_1{
        *this,
        value
    };

    explicit TestPage(const char *s) :
        Page{s} {}

};

}
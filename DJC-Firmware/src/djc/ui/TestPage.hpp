#pragma once

#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/math/time/Timer.hpp>

#include "djc/UI.hpp"

namespace djc {

struct TestPage : UI::Page {
    using Boiler = UI::Labeled<UI::ComboBox<int, 3>>;

    Boiler boiler{
        *this,
        "Boiler",
        Boiler::Impl{
            {{
                {"ice", 1},
                {"water", 20},
                {"steam", 300},
            }},
        }};

    float value{12.3456};
    Boiler::Impl::Value item{0};

    UI::Display<int> display_2{*this, item};

    UI::SpinBox<float> spin_box{
        *this,
        value,
        0.1f,
        UI::SpinBox<float>::Mode::Arithmetic};

    UI::Button button{*this, "button"};

    UI::CheckBox check_box{*this};

    UI::Display<float> display_1{*this, value};

    explicit TestPage(const char *s) :
        Page{s} {

        button.on_click = [this]() {
            kf_Logger_debug("button click");
            value = -value * 1.4f;
        };

        boiler.impl.change_handler = [this](int v) {
            item = v;
            kf_Logger_debug("state: %d", v);
        };

        spin_box.change_handler = [this](float v) {
            value = v;
            kf_Logger_debug("value: %f", v);
        };

        check_box.change_handler = [](bool state) {
            kf_Logger_debug("state: %s", state ? "ON" : "OFF");
        };
    }
};

}// namespace djc
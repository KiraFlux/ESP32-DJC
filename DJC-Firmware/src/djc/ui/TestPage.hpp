#pragma once

#include "djc/UI.hpp"
#include "djc/ui/MainPage.hpp"


namespace djc {

/// @brief Test page demonstrating various UI widgets
struct TestPage : UI::Page {
    using Boiler = UI::Labeled <UI::ComboBox<int, 3>>;

    // UI Widgets
    Boiler boiler{
        *this,
        "Boiler",
        Boiler::Impl{
            {
                {
                    {"ice", 1},
                    {"water", 20},
                    {"steam", 300},
                }
            },
        }
    };

    float value{12.3456};
    Boiler::Impl::Value item{0};

    UI::Display<int> display_2{*this, item};
    UI::SpinBox<float> spin_box{*this, value, 0.1f, UI::SpinBox<float>::Mode::Arithmetic};
    UI::Button button{*this, "button"};
    UI::CheckBox check_box{*this};
    UI::Display<float> display_1{*this, value};

    // Character set test display
    kf::StringView text_view{
        "" // Extended ASCII characters for display testing
        "\xF0#0#\xF1#1#\xF2#2#\xF3#3#\xF4#4#\xF5#5#\xF6#6#\xF7#7#\n"
        "\xF8#8#\xF9#9#\xFA#A#\xFB#B#\xFC#C#\xFD#D#\xFE#E#\xFF#F#\n"
        "\xB0 0 \xB1 1 \xB2 2 \xB3 3 \xB4 4 \xB5 5 \xB6 6 \xB7 7 \n"
        "\xB8 8 \xB9 9 \xBB A \xBB B \xBC C \xBD D \xBE E \xBF F "
    };

    UI::Display <kf::StringView> display_text{*this, text_view};

    explicit TestPage(kf::StringView title) :
        Page{title} {
        link(MainPage::instance());

        // Widget event handlers
        button.on_click = [this]() {
            kf_Logger_debug("Button clicked");
            value = -value * 1.4f;
        };

        boiler.impl.change_handler = [this](int v) {
            item = v;
            kf_Logger_debug("Boiler state changed: %d", v);
        };

        spin_box.change_handler = [this](float v) {
            value = v;
            kf_Logger_debug("Spin box value: %.3f", v);
        };

        check_box.change_handler = [](bool state) {
            kf_Logger_debug("Checkbox state: %s", state ? "ON" : "OFF");
        };
    }
};

} // namespace djc
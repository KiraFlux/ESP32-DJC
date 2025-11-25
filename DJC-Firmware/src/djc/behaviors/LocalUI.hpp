#pragma once

#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>
#include <kf/Logger.hpp>

#include <kf/UI.hpp>
#include <kf/ui/TextRender.hpp>


namespace djc {

struct LocalUI : kf::tools::Singleton<LocalUI>, kf::sys::Behavior {
    friend struct Singleton<LocalUI>;

    /// @brief Определение Textual UI
    using TUI = kf::UI<kf::ui::TextRender>;

    std::array<char, 250> text_buffer{"null"};
    kf::sys::TextComponent text_component{text_buffer.data()};

    struct TestPage : TUI::Page {
        float value{12.3456};

        using Boiler = TUI::Labeled <TUI::ComboBox<int, 3>>;

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

        TUI::Display<int> display_2{
            *this,
            item
        };

        TUI::SpinBox<float> spin_box{
            *this,
            value,
            0.1f,
            TUI::SpinBox<float>::Mode::Arithmetic
        };

        TUI::Button button{
            *this,
            "button",
            [this]() {
                kf_Logger_debug("button click");
                value = -value * 1.4f;
            }
        };

        TUI::CheckBox check_box{
            *this,
            [](bool state) {
                kf_Logger_debug("state: %s", state ? "ON" : "OFF");
            }
        };

        TUI::Display<float> display_1{
            *this,
            value
        };

        explicit TestPage(const char *s) :
            Page{s} {}

    }
        test_page_1{"p1"}, test_page_2{"p2"};

    void updateLayout(kf::gfx::Canvas &root) override {
        text_component.canvas = root;

        auto &ui = TUI::instance();
        ui.bindPage(test_page_1);

        test_page_1.link(test_page_2);

        auto &render_settings = ui.getRenderSettings();
        render_settings.buffer = {reinterpret_cast<kf::u8 *>(text_buffer.data()), text_buffer.size()};
        render_settings.rows = root.widthInGlyph();
        render_settings.cols = root.heightInGlyph();
        render_settings.on_render_finish = [](const kf::slice<const kf::u8> &str) {
            Serial.printf("render (%d): %s\n", str.size(), str.data());
        };

        ui.addEvent(kf::ui::Event{kf::ui::Event::Type::Update});
    }

    void onEntry() override {
        using E = TUI::Event;

        static auto &ui = TUI::instance();

        auto &periphery = djc::Periphery::instance();
        periphery.right_button.handler = []() {
            ui.addEvent(E{E::Type::WidgetClick});
        };
        periphery.joystick_listener.handler = [](kf::JoystickListener::Direction dir) {
            using D = kf::JoystickListener::Direction;
            auto translate = [](D direction) -> E {
                using T = E::Type;
                switch (direction) {
                    case D::Up:return E{T::PageCursorMove, -1};
                    case D::Down:return E{T::PageCursorMove, +1};
                    case D::Left:return E{T::WidgetValueChange, +1};
                    case D::Right:return E{T::WidgetValueChange, -1};
                    default:return E{T::None};
                }
            };

            const auto event = translate(dir);
            if (E::Type::None != event.type()) {
                ui.addEvent(event);
            }
        };
    }

    void update() override {
        auto &ui = TUI::instance();
        ui.poll();

        auto &periphery = djc::Periphery::instance();
        periphery.right_button.poll();
        periphery.joystick_listener.poll();
    }

    explicit LocalUI() {
        addComponent(text_component);
    }
};

}
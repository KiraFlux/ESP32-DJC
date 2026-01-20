#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/math/time/Timer.hpp>
#include <kf/gfx.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/memory/ArrayString.hpp>

#include "djc/Periphery.hpp"
#include "djc/UI.hpp"
#include "djc/ui/MainPage.hpp"
#include "djc/ui/MavLinkControlPage.hpp"
#include "djc/ui/TestPage.hpp"


static auto &ui = djc::UI::instance();

static auto &periphery = djc::Periphery::instance();

static djc::MainPage main_page{};

static bool menu_navigation_enabled{true};

void setup() {
    // Logging setup
    Serial.begin(115200);
    kf_Logger_setWriter([](kf::Slice<const char> str) { Serial.write(str.data(), str.size()); });

    // Periphery setup
    (void) periphery.init(); // ignoring failure
    periphery.left_joystick.axis_x.inverted = true;
    periphery.right_joystick.axis_y.inverted = true;
    using D = kf::JoystickListener::Direction;
    using E = djc::UI::Event;

    periphery.right_joystick_listener.handler = [](D direction) {
        switch (direction) {
            case D::Home://
                break;
            case D::Up://
                ui.addEvent(E::PageCursorMove(-1));
                break;
            case D::Down://
                ui.addEvent(E::PageCursorMove(+1));
                break;
            case D::Left://
                ui.addEvent(E::WidgetValueChange(-1));
                break;
            case D::Right://
                ui.addEvent(E::WidgetValueChange(+1));
                break;
        }
    };
    periphery.right_button.handler = []() {
        ui.addEvent(E::WidgetClick());
    };
    periphery.left_button.handler = []() {
        menu_navigation_enabled ^= 1;
        ui.addEvent(E::Update());
    };

    periphery.left_joystick.calibrate(100);
    periphery.right_joystick.calibrate(100);

    // Graphics setup
    constexpr auto pixel_format{decltype(periphery.display_driver)::pixel_format};
    auto &display = periphery.display_driver;

    static kf::gfx::Canvas<pixel_format> canvas{
        kf::gfx::DynamicImage<pixel_format>{
            // buffer: data, stride
            display.buffer().data(), display.width(),
            // size: width, height
            display.width(), display.height(),
            // offset: x, y
            0, 0
        },
        // font
        kf::gfx::fonts::gyver_5x7_en
    };
    canvas.setAutoNextLine(true);

    // Textual Render setup
    static kf::ArrayString<256> text_buffer{};

    ui.renderSettings() = {
        .row_max_length = canvas.widthInGlyphs(),
        .rows_total = canvas.heightInGlyphs(),
        .buffer = kf::Slice<char>{text_buffer.data(), text_buffer.size()},
        .on_render_finish = [](kf::StringView str) {
            canvas.fill();
            canvas.text(0, 0, str.data());

            if (not menu_navigation_enabled) {
                const auto h = static_cast<kf::Pixel>(canvas.glyphHeight() / 2);
                canvas.line(0, h, canvas.maxX(), h);
            }

            periphery.display_driver.send();
        }
    };

    // prepare UI

    static djc::MavLinkControlPage mav_link_control{};
    main_page.link(mav_link_control);

    static djc::TestPage test_page1{"Test 1"}, test_page2{"Test 2"};
    main_page.link(test_page1);
    main_page.link(test_page2);

    ui.bindPage(main_page);
    ui.addEvent(djc::UI::Event::Update());
}

void loop() {
    static kf::Timer poll_timer{static_cast<kf::Hertz>(50)};

    if (not poll_timer.ready(millis())) { return; }

    ui.poll();
    periphery.left_button.poll();

    if (not menu_navigation_enabled) { return; }

    periphery.right_button.poll();
    periphery.right_joystick_listener.poll();

}
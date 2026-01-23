#include <Arduino.h>

#include <kf/Logger.hpp>
#include <kf/gfx.hpp>
#include <kf/math/time/Timer.hpp>
#include <kf/memory/StringView.hpp>

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
    kf_Logger_setWriter([](kf::StringView str) { Serial.write(str.data(), str.size()); });

    // Periphery setup
    (void) periphery.init();// ignoring failure
    periphery.left_joystick.axis_x.inverted = true;
    periphery.right_joystick.axis_y.inverted = true;

    periphery.left_joystick.calibrate(100);
    periphery.right_joystick.calibrate(100);

    // Graphics setup
    constexpr auto pixel_format{decltype(periphery.display)::pixel_format};

    static kf::gfx::Canvas<pixel_format> root_canvas{
        kf::gfx::DynamicImage<pixel_format>{
            // buffer: data, stride
            periphery.display.buffer().data(), periphery.display.width(),
            // size: width, height
            periphery.display.width(), periphery.display.height(),
            // offset: x, y
            0, 0},
        // font
        kf::gfx::fonts::gyver_5x7_en};
    root_canvas.setAutoNextLine(true);

    // Textual Render setup
    ui.renderConfig() = {
        .on_render_finish = [](kf::StringView str) {
            root_canvas.fill();
            root_canvas.text(0, 0, str.data());

            if (not menu_navigation_enabled) {
                const auto h = static_cast<kf::Pixel>(root_canvas.glyphHeight() / 2);
                root_canvas.line(0, h, root_canvas.maxX(), h);
            }

            periphery.display.send();
        },
        .row_max_length = root_canvas.widthInGlyphs(),
        .rows_total = root_canvas.heightInGlyphs(),
    };

    // UI setup
    static djc::MavLinkControlPage mav_link_control{};
    main_page.link(mav_link_control);

    static djc::TestPage test_page1{"Test 1"}, test_page2{"Test 2"};
    main_page.link(test_page1);
    main_page.link(test_page2);

    ui.bindPage(main_page);
    ui.addEvent(djc::UI::Event::update());
}

void loop() {
    using Event = djc::UI::Event;

    static constexpr Event direction_to_event_table[]{
        Event::pageCursorMove(-1),// 0: Up
        Event::pageCursorMove(+1),// 1: Down
        Event::widgetValue(-1),   // 2: Left
        Event::widgetValue(+1),   // 3: Right
    };

    constexpr kf::Milliseconds loop_period{1000 / 50};
    delay(loop_period);

    periphery.left_button.poll();
    periphery.right_button.poll();
    periphery.right_joystick_listener.poll(millis());

    if (periphery.left_button.clicked()) {
        menu_navigation_enabled ^= 1;
        ui.addEvent(Event::update());
    }

    if (periphery.right_button.clicked()) {
        if (menu_navigation_enabled) {
            ui.addEvent(Event::widgetClick());
        }
    }

    if (periphery.right_joystick_listener.changed()) {
        const auto direction = periphery.right_joystick_listener.direction();
        if (direction != kf::JoystickListener::Direction::Home) {
            if (menu_navigation_enabled) {
                ui.addEvent(direction_to_event_table[static_cast<kf::u8>(direction)]);
            }
        }
    }

    ui.poll();
}
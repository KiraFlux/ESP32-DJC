#pragma once

#include <kf/pattern/Singleton.hpp>
#include <kf/gfx.hpp>

#include "djc/Periphery.hpp"
#include "djc/UI.hpp"


namespace djc {

struct Device : kf::Singleton<Device> {
    friend struct Singleton<Device>;

    using Event = djc::UI::Event;

    struct ControllerValues {
        kf::f32 left_x{};
        kf::f32 left_y{};
        kf::f32 right_x{};
        kf::f32 right_y{};

        void reset() {
            *this = ControllerValues{};
        }
    };

    static constexpr auto pixel_format{Periphery::SelectedDisplayDriver::pixel_format};

private:

    Periphery periphery{};

    kf::gfx::Canvas<pixel_format> root_canvas{};

    ControllerValues controller_values{};

    djc::UI &ui = djc::UI::instance();

    bool menu_navigation_enabled{true};

public:

    // setup and poll

    void setupPeriphery() noexcept {
        (void) periphery.init();// ignoring failure (for now...)
        tune(100); // tune analog axis with 100 samples
    }

    void setupGraphics() noexcept {
        root_canvas = kf::gfx::Canvas<pixel_format>{
            kf::gfx::DynamicImage<pixel_format>{
                // buffer: data, stride
                periphery.display.buffer().data(), periphery.display.width(),
                // size: width, height
                periphery.display.width(), periphery.display.height(),
                // offset: x, y
                0, 0
            },
            // font
            kf::gfx::fonts::gyver_5x7_en
        };
        root_canvas.setAutoNextLine(true);
    }

    void setupUiRenderConfig(UI::RenderConfig &config) noexcept {
        config.on_render_finish = [this](kf::StringView str) {
            root_canvas.fill();
            onRender(str);
            periphery.display.send();
        };
        config.row_max_length = root_canvas.widthInGlyphs();
        config.rows_total = root_canvas.heightInGlyphs();
    }

    void poll(kf::Milliseconds now) noexcept {
        periphery.left_button.poll(now);
        if (periphery.left_button.clicked()) {
            onLeftButtonClick();
        }

        if (menu_navigation_enabled) {
            periphery.right_button.poll(now);
            if (periphery.right_button.clicked()) {
                onNavigationRightButtonClick();
            }

            periphery.right_joystick_listener.poll(now);
            if (periphery.right_joystick_listener.changed()) {
                const auto direction = periphery.right_joystick_listener.direction();
                if (direction != kf::JoystickListener::Direction::Home) {
                    onNavigationRightJoystickDirection(direction);
                }
            }
        } else {
            controller_values.left_x = periphery.left_joystick.axis_x.read();
            controller_values.left_y = periphery.left_joystick.axis_y.read();
            controller_values.right_x = periphery.right_joystick.axis_x.read();
            controller_values.right_y = periphery.right_joystick.axis_y.read();
        }
    }

    // utility

    kf_nodiscard const ControllerValues &controllerValues() const noexcept { return controller_values; }

    kf_nodiscard kf::Option<kf::EspNow::Peer> &espnowPeer() noexcept { return periphery.espnow_peer; }

private:

    // display

    void onRender(kf::StringView str) noexcept {
        kf::Pixel y{0};

        if (not menu_navigation_enabled) {
            constexpr kf::StringView s{"\xB6""Controller Mode\n"};
            root_canvas.text(0, 0, s.data());
            y = root_canvas.glyphHeight();
        }

        root_canvas.text(0, y, str.data());
    }

    // input

    void onLeftButtonClick() noexcept {
        menu_navigation_enabled = not menu_navigation_enabled;
        controller_values.reset();
        ui.addEvent(Event::update());
    }

    void onNavigationRightButtonClick() const noexcept {
        ui.addEvent(Event::widgetClick());
    }

    void onNavigationRightJoystickDirection(kf::JoystickListener::Direction direction) const noexcept {
        static constexpr Event event_from_direction[4]{
            Event::pageCursorMove(-1),// 0: Up
            Event::pageCursorMove(+1),// 1: Down
            Event::widgetValue(-1),   // 2: Left
            Event::widgetValue(+1),   // 3: Right
        };

        ui.addEvent(event_from_direction[static_cast<kf::u8>(direction)]);
    }

    //

    void tune(kf::u16 samples) noexcept {
        kf::AnalogAxis::AxisTuner lx{periphery.config.left_joystick.x, samples};
        kf::AnalogAxis::AxisTuner ly{periphery.config.left_joystick.y, samples};
        kf::AnalogAxis::AxisTuner rx{periphery.config.right_joystick.x, samples};
        kf::AnalogAxis::AxisTuner ry{periphery.config.right_joystick.y, samples};

        lx.start();
        ly.start();
        rx.start();
        ry.start();

        while (lx.running() or ly.running() or rx.running() or ry.running()) {
            lx.poll(periphery.left_joystick.axis_x.readRaw());
            ly.poll(periphery.left_joystick.axis_y.readRaw());
            rx.poll(periphery.right_joystick.axis_x.readRaw());
            ry.poll(periphery.right_joystick.axis_y.readRaw());
            delay(1);
        }
    }
};

}
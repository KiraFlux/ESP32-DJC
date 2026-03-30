// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/gfx/Canvas.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/mixin/Singleton.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/Periphery.hpp"
#include "djc/ui/UI.hpp"

namespace djc {

/// @brief Main device controller for ESP32-DJC
struct Device : kf::mixin::Singleton<Device>, kf::mixin::TimedPollable<Device> {

    /// @brief Controller values for manual control mode
    struct ControllerValues {
        kf::f32 left_x{};
        kf::f32 left_y{};
        kf::f32 right_x{};
        kf::f32 right_y{};

        void reset() { *this = ControllerValues{}; }
    };

    using PixelImpl = DisplayDriver::PixelImpl;

    // Setup methods
    void setupPeriphery() noexcept {
        (void) periphery.init();// Ignoring failure for now
        tune(100);              // Tune analog axes with 100 samples
    }

    void setupGraphics() noexcept {
        root_canvas = kf::gfx::Canvas<PixelImpl>{
            kf::image::DynamicImage<PixelImpl>{periphery.display.image()},
            kf::gfx::fonts::gyver_5x7_en};
        root_canvas.autoNextLine(true);
    }

    void setupRender(ui::UI::RenderConfig &config) noexcept {
        config.callback([this](kf::memory::StringView str) {
            root_canvas.fill();
            onRender(str);
            (void) periphery.display.send();
        });
        config.row_max_length = root_canvas.widthInGlyphs();
        config.rows_total = root_canvas.heightInGlyphs();
    }

    // Accessors
    const ControllerValues &controllerValues() const noexcept { return controller_values; }

    kf::Option<kf::network::EspNow::Peer> &espnowPeer() noexcept { return periphery.espnow_peer; }

    bool isNavigationEnabled() const noexcept { return menu_navigation_enabled; }

private:
    Periphery periphery{};
    kf::gfx::Canvas<PixelImpl> root_canvas{};
    ControllerValues controller_values{};
    ui::UI &ui{ui::UI::instance()};
    bool menu_navigation_enabled{true};

    // Display rendering
    void onRender(kf::memory::StringView str) noexcept {
        kf::math::Pixels y{0};

        // Show mode indicator
        if (not menu_navigation_enabled) {
            constexpr kf::memory::StringView mode_indicator{"\xB6"
                                                            "Controller Mode\n"};
            root_canvas.text(0, 0, mode_indicator.data());
            y = root_canvas.glyphHeight();
        }

        root_canvas.text(0, y, str.data());
    }

    // Input event handlers
    void onLeftButtonClick() noexcept {
        menu_navigation_enabled = not menu_navigation_enabled;
        controller_values.reset();
        ui.addEvent(ui::UI::Event::update());
    }

    void onNavigationRightButtonClick() const noexcept {
        ui.addEvent(ui::UI::Event::widgetClick());
    }

    void onNavigationRightJoystickDirection(JoystickListener::Direction direction) const noexcept {
        static constexpr ui::UI::Event event_from_direction[4] = {
            ui::UI::Event::pageCursorMove(-1),// Up
            ui::UI::Event::pageCursorMove(+1),// Down
            ui::UI::Event::widgetValue(-1),   // Left
            ui::UI::Event::widgetValue(+1),   // Right
        };

        ui.addEvent(event_from_direction[static_cast<kf::u8>(direction)]);
    }

    // Analog axis calibration
    void tune(kf::u16 samples) noexcept {
        Joystick::Tuner left_tuner{periphery.config.left_joystick, periphery.left_joystick, samples};
        Joystick::Tuner right_tuner{periphery.config.right_joystick, periphery.right_joystick, samples};

        left_tuner.reset();
        right_tuner.reset();

        // Poll all axes until calibration complete
        while (left_tuner.running() or right_tuner.running()) {
            left_tuner.poll();
            right_tuner.poll();
            delay(1);
        }
    }

    // impl

    KF_IMPL_TIMED_POLLABLE(Device);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        // Left button (mode toggle)
        periphery.left_button.poll(now);
        if (periphery.left_button.clicked()) {
            onLeftButtonClick();
        }

        if (menu_navigation_enabled) {
            // Navigation mode: UI control
            periphery.right_button.poll(now);
            if (periphery.right_button.clicked()) {
                onNavigationRightButtonClick();
            }

            periphery.right_joystick_listener.poll(now);
            if (periphery.right_joystick_listener.changed()) {
                const auto direction = periphery.right_joystick_listener.direction();
                if (direction != JoystickListener::Direction::Home) {
                    onNavigationRightJoystickDirection(direction);
                }
            }
        } else {
            // Control mode: read joystick values
            controller_values.left_x = periphery.left_joystick.axis_x.read();
            controller_values.left_y = periphery.left_joystick.axis_y.read();
            controller_values.right_x = periphery.right_joystick.axis_x.read();
            controller_values.right_y = periphery.right_joystick.axis_y.read();
        }
    }
};

}// namespace djc
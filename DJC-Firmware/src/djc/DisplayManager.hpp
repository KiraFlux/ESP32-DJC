// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/gfx/Canvas.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/DeviceState.hpp"
#include "djc/Keyboard.hpp"
#include "djc/prelude.hpp"
#include "djc/ui/UI.hpp"

namespace djc {

struct DisplayManager final : kf::mixin::NonCopyable, kf::mixin::Initable<DisplayManager, void> {

    explicit DisplayManager(DisplayDriver &display, const DeviceState &device_state, const Keyboard &keyboard) noexcept :
        _display{display}, _device_state{device_state}, _keyboard{keyboard} {}

private:
    DisplayDriver &_display;
    const DeviceState &_device_state;
    const Keyboard &_keyboard;
    kf::gfx::Canvas<DisplayDriver::PixelImpl> _canvas{};

    KF_IMPL_INITABLE(DisplayManager, void);
    void initImpl() noexcept {
        _canvas = kf::gfx::Canvas<DisplayDriver::PixelImpl>{
            kf::image::DynamicImage<DisplayDriver::PixelImpl>{_display.image()},
            kf::gfx::fonts::gyver_5x7_en,
        };
        _canvas.autoNextLine(true);

        auto &config = ui::UI::instance().renderConfig();
        config.callback([this](kf::memory::StringView str) {
            _canvas.fill();
            onRender(str);
            (void) _display.send();
        });
        config.row_max_length = _canvas.widthInGlyphs();
        config.rows_total = _canvas.heightInGlyphs() - 1;
    }

    void onRender(kf::memory::StringView str) noexcept {
        if (_device_state.keyboardInputEnabled()) {

            _canvas.text(0, 0, kf::memory::ArrayString<32>::formatted(
                "\xBC\xF0Text Input: %d / %d\x80\n", 
                _keyboard.available(),
                _keyboard.text().size()).data()
            );

            _canvas.text(0, _canvas.glyphHeight(), _keyboard.text().data());

            const auto y = static_cast<kf::math::Pixels>(_canvas.maxY() - _canvas.glyphHeight());

            kf::memory::ArrayString<64> keys_buffer{};
            (void) keys_buffer.push('\xB8');

            for (auto i = 0; i < Keyboard::keys.size(); i += 1) {
                const bool selected{i == _keyboard.selectedButtonIndex()};
                
                if (selected) {
                    (void) keys_buffer.push('\xF0');
                    (void) keys_buffer.push('\xBF');
                }

                (void) keys_buffer.push(Keyboard::keys[i].value);
                (void) keys_buffer.push(' ');

                if (selected) {
                    (void) keys_buffer.push('\x80');
                }
            
            }

            _canvas.text(0, y, keys_buffer.data());

            return;
        }

        // Show mode indicator
        if (_device_state.controlEnabled()) {
            auto y = static_cast<kf::math::Pixels>(_canvas.maxY() - _canvas.glyphHeight());
            _canvas.text(0, y, "\xB2\xF0 Control Enabled");
        }

        _canvas.text(0, 0, str.data());
    }
};

}// namespace djc
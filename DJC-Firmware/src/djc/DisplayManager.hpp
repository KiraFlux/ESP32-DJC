// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
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
    using P = kf::gfx::Palette<DisplayDriver::PixelImpl>;

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
            onRender(str);
            (void) _display.send();
        });
        config.row_max_length = _canvas.widthInGlyphs();
        config.rows_total = _canvas.heightInGlyphs() - 1;
    }

    void onRender(kf::memory::StringView str) noexcept {
        _canvas.background(P::black);
        _canvas.foreground(P::white);
        
        _canvas.fill();
        
        if (_device_state.keyboardInputEnabled()) {
            _canvas.text(0, 0, kf::memory::ArrayString<32>::formatted("\xBC\xF0Text Input: %d / %d\x80\n", _keyboard.available(), _keyboard.text().size()).data());
            _canvas.text(0, _canvas.glyphHeight(), _keyboard.text().data());

            renderKeyboard();
            return;
        }

        // Control mode overlay
        if (_device_state.controlEnabled()) {
            _canvas.text(0, static_cast<kf::math::Pixels>(_canvas.maxY() - _canvas.glyphHeight()), "\xB2\xF0 Control Enabled");
        }

        _canvas.background(P::black);
        _canvas.foreground(P::white);
        _canvas.text(0, 0, str.data());
    }

    void renderKeyboard() noexcept {
        const auto key_height = _canvas.glyphHeight();
        const auto start_y = _canvas.maxY() - key_height * _keyboard.rowsTotal();
        const auto longest_row = Keyboard::keys_in_row[0];

        const auto key_width = _canvas.width() / longest_row;
        const auto glyph_offset = (key_width - _canvas.glyphWidth()) / 2;

        char c[2]{0, 0};
        _canvas.background(P::bright_black);
        for (auto row = 0; row < _keyboard.rowsTotal(); row += 1) {
            const auto y = start_y + row * key_height;
            const auto cols = Keyboard::keys_in_row[row];

            const auto x_offset = ((longest_row - cols) * key_width) / 2;

            for (auto col = 0; col < cols; col += 1) {
                const auto x = col * key_width + x_offset;

                _canvas.foreground(P::bright_black);
                _canvas.rect(x, y, x + key_width, y + key_height, true);

                _canvas.foreground((row == _keyboard.row() and col == _keyboard.col()) ? P::white : P::black);

                c[0] = Keyboard::keys[Keyboard::selectedIndex(row, col)].value;
                _canvas.text(x + glyph_offset, y, c);
            }
        }
    }
};

}// namespace djc
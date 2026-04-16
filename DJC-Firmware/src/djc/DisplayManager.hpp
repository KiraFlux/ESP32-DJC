// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/Control.hpp"
#include "djc/input/VirtualKeyboard.hpp"
#include "djc/prelude.hpp"
#include "djc/ui/UI.hpp"

namespace djc {

struct DisplayManager final : kf::mixin::NonCopyable, kf::mixin::Initable<DisplayManager, void> {

    explicit DisplayManager(DisplayDriver &display, const Control &control) noexcept :
        _display{display}, _control{control} {}

private:
    using P = kf::gfx::Palette<DisplayDriver::PixelImpl>;

    inline static const auto &virtual_keyboard = input::VirtualKeyboard::instance();

    DisplayDriver &_display;
    const Control &_control;
    kf::gfx::Canvas<DisplayDriver::PixelImpl> _canvas{};

    void onRender(kf::memory::StringView str) noexcept {
        _canvas.background(P::black);
        _canvas.foreground(P::white);

        _canvas.fill();

        if (virtual_keyboard.active()) {
            renderVirtualKeyboard();
        } else {
            renderUi(str);
        }
    }

    void renderUi(kf::memory::StringView str) noexcept {
        // Control mode overlay
        if (_control.enabled()) {
            const auto y = static_cast<kf::math::Pixels>(_canvas.maxY() - _canvas.glyphHeight());

            const auto overlay = kf::memory::ArrayString<64>::formatted(
                "\xB6\xF0""Control [%s]",
                (_control.connected() ? EspNow::stringFromMac(_control.activeMac().value()).data() : "Disconnected"));
            _canvas.text(0, y, overlay.data());
        }

        _canvas.background(P::black);
        _canvas.foreground(P::white);
        _canvas.text(0, 0, str.data());
    }

    void renderVirtualKeyboard() noexcept {
        const auto longest_row = input::VirtualKeyboard::rows[0].size();
        const auto key_width = _canvas.width() / longest_row;
        const auto key_height = _canvas.glyphHeight();
        const auto keyboard_offset_y = _canvas.maxY() - key_height * virtual_keyboard.rowsTotal();
        const auto glyph_offset_x = (key_width - _canvas.glyphWidth()) / 2;

        char c[2]{0, 0};

        _canvas.text(0, 0, kf::memory::ArrayString<32>::formatted("\xBC\xF0Text Input: %d / %d\x80\n", virtual_keyboard.available(), virtual_keyboard.text().size()).data());
        _canvas.text(0, _canvas.glyphHeight(), virtual_keyboard.text().data());

        _canvas.background(P::bright_black);
        _canvas.foreground(P::bright_black);
        _canvas.rect(0, keyboard_offset_y, _canvas.maxX(), _canvas.maxY(), true);

        for (auto row = 0; row < virtual_keyboard.rowsTotal(); row += 1) {
            const auto y = keyboard_offset_y + row * key_height;
            const auto cols = input::VirtualKeyboard::rows[row].size();

            const auto x_offset = ((longest_row - cols) * key_width) / 2;

            for (auto col = 0; col < cols; col += 1) {
                const auto x = col * key_width + x_offset;

                if (row == virtual_keyboard.cursorRow() and col == virtual_keyboard.cursorCol()) {
                    _canvas.foreground(P::blue);
                    _canvas.rect(x, y, x + key_width, y + key_height - 1, true);

                    _canvas.background(P::blue);
                    _canvas.foreground(P::bright_white);
                } else {
                    _canvas.background(P::bright_black);
                    _canvas.foreground(P::black);
                }

                const auto &key = input::VirtualKeyboard::keyAt(row, col);
                if (key.kind == input::VirtualKeyboard::Key::Kind::Common) {
                    c[0] = key.value(virtual_keyboard.shifted());
                } else {
                    c[0] = '?';
                }

                _canvas.text(x + glyph_offset_x, y, c);
            }
        }
    }

    // impl

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
};

}// namespace djc
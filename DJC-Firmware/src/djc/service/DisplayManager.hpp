// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/drivers/display/DisplayDriver.hpp>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/primitives.hpp>

#include "djc/transport/TransportLink.hpp"
#include "djc/ui/VirtualKeyboard.hpp"

#include "djc/service/Service.hpp"

namespace djc::service {

/// @brief Service that manages display rendering, including UI and virtual keyboard overlay.
template<typename I> struct DisplayManager final :

    Service<DisplayManager<I>>,
    kf::mixin::Initable<DisplayManager<I>, void>

{
    KF_CHECK_IMPL(I, ::kf::drivers::display::DisplayDriverTag);

    using DisplayDriverImpl = I;
    using Pixel = typename DisplayDriverImpl::PixelImpl;
    using Canvas = typename kf::gfx::Canvas<Pixel>;
    using Palette = kf::gfx::Palette<Pixel>;

    explicit DisplayManager(DisplayDriverImpl &display_driver, const transport::TransportLink &transport_link, const ui::VirtualKeyboard &virtual_keyboard) noexcept :
        _display_driver{display_driver}, _transport_link{transport_link}, _virtual_keyboard{virtual_keyboard} {}

    [[nodiscard]] auto canvas() const noexcept -> const kf::Option<Canvas> & {
        return _canvas;
    }

    void showConnectionStatusOverlay(bool show) noexcept {
        _show_connection_status_overlay = show;
    }

    void onRender(kf::memory::StringView str) noexcept {
        if (_canvas.isNone()) { return; }

        _canvas.unwrap().background(Palette::black);
        _canvas.unwrap().foreground(Palette::white);

        _canvas.unwrap().fill();

        if (_virtual_keyboard.active()) {
            renderVirtualKeyboard();
        } else {
            renderUi(str);
        }

        (void) _display_driver.send();
    }

private:
    DisplayDriverImpl &_display_driver;
    const transport::TransportLink &_transport_link;
    const ui::VirtualKeyboard &_virtual_keyboard;
    kf::Option<Canvas> _canvas{kf::none};
    bool _show_connection_status_overlay{false};

    void renderUi(kf::memory::StringView str) noexcept {
        auto &canvas = _canvas.unwrap();

        if (_show_connection_status_overlay) {
            const auto y = static_cast<kf::math::Pixels>(canvas.maxY() - canvas.font().heightTotal());
            const auto overlay = _transport_link.connected() ? _transport_link.activePeerAddress().unwrap().toString().data() : "Disconnected";

            canvas.background(Palette::bright_blue);
            canvas.foreground(Palette::black);
            canvas.text(0, y, overlay);
        }

        canvas.background(Palette::black);
        canvas.foreground(Palette::white);
        canvas.text(0, 0, str.data());
    }

    void renderVirtualKeyboard() noexcept {
        auto &canvas = _canvas.unwrap();

        const auto longest_row = ui::VirtualKeyboard::rows[0].size();
        const auto key_width = canvas.width() / longest_row;
        const auto key_height = canvas.font().heightTotal();
        const auto keyboard_offset_y = canvas.maxY() - key_height * _virtual_keyboard.rowsTotal();
        const auto glyph_offset_x = (key_width - canvas.font().widthTotal()) / 2;

        canvas.text(0, 0, kf::memory::StaticString<32>::formatted("\xBC\xF0Text Input: %d / %d\x80\n", _virtual_keyboard.available(), _virtual_keyboard.text().size()).data());
        canvas.text(0, canvas.font().heightTotal(), _virtual_keyboard.text().data());

        canvas.background(Palette::bright_black);
        canvas.foreground(Palette::bright_black);
        canvas.rect(0, keyboard_offset_y, canvas.maxX(), canvas.maxY(), true);

        for (auto row = 0; row < _virtual_keyboard.rowsTotal(); row += 1) {
            const auto y = keyboard_offset_y + row * key_height;
            const auto cols = ui::VirtualKeyboard::rows[row].size();

            const auto x_offset = ((longest_row - cols) * key_width) / 2;

            for (auto col = 0; col < cols; col += 1) {
                const auto x = col * key_width + x_offset;

                if (row == _virtual_keyboard.cursorRow() and col == _virtual_keyboard.cursorCol()) {
                    canvas.foreground(Palette::blue);
                    canvas.rect(x, y, x + key_width, y + key_height - 1, true);

                    canvas.background(Palette::blue);
                    canvas.foreground(Palette::bright_white);
                } else {
                    canvas.background(Palette::bright_black);
                    canvas.foreground(Palette::black);
                }

                const auto &key = ui::VirtualKeyboard::keyAt(row, col);

                canvas.glyph(x + glyph_offset_x, y, key.isCommon() ? key.value(_virtual_keyboard.shifted()) : '?');
            }
        }
    }

    // impl
    using This = DisplayManager<I>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        Canvas canvas{
            kf::image::DynamicImage<Pixel>{_display_driver.image()},
            typename Canvas::State{
                .active_font = kf::someRef(kf::gfx::fonts::gyver_5x7_en),
                .auto_next_line = true,
            },
        };
        _canvas = kf::some(std::move(canvas));
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {}
};

}// namespace djc::service
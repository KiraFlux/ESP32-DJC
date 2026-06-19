// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/memory/StaticString.hpp>

#include "djc/mixin/ServiceOwner.hpp"
#include "djc/service/DisplayService.hpp"
#include "djc/system/System.hpp"
#include "djc/ui/VirtualKeyboard.hpp"

namespace djc::system {

/// @brief System managing display output, canvas, and virtual keyboard.
template<typename I> struct GraphicsSystem :

    System<GraphicsSystem<I>, void(I &)>,
    mixin::ServiceOwner<service::DisplayService<I>>

{
    using DisplayDriverImpl = I;
    using DisplayServiceImpl = service::DisplayService<DisplayDriverImpl>;

    using Pixel = typename DisplayDriverImpl::PixelImpl;
    using Color = typename Pixel::ColorType;
    using Canvas = typename kf::gfx::Canvas<Pixel>;
    using Palette = kf::gfx::Palette<Pixel>;

    explicit GraphicsSystem(I &display_driver, const ui::VirtualKeyboard &virtual_keyboard) noexcept :
        mixin::ServiceOwner<DisplayServiceImpl>{DisplayServiceImpl{display_driver}}, _virtual_keyboard{virtual_keyboard} {}

    [[nodiscard]] auto canvas() const noexcept -> const kf::Option<Canvas> & {
        return _canvas;
    }

    void overlay(kf::memory::StringView new_overlay, Color color) noexcept {
        _overlay = new_overlay;
        _overlay_color = color;
        this->service().requestSend();
    }

    void onRender(kf::memory::StringView str) noexcept {
        if (_canvas.isNone()) { return; }
        auto &canvas = _canvas.unwrap();

        _canvas.unwrap().background(Palette::black);
        _canvas.unwrap().foreground(Palette::white);

        _canvas.unwrap().fill();

        if (_virtual_keyboard.active()) {
            renderVirtualKeyboard(canvas);
        } else {
            renderUi(canvas, str);
        }

        this->service().requestSend();
    }

private:
    static constexpr auto overlay_text_padding{1};

    const ui::VirtualKeyboard &_virtual_keyboard;
    kf::Option<Canvas> _canvas{kf::none};
    kf::memory::StringView _overlay{};
    Color _overlay_color{};

    void renderUi(Canvas &canvas, kf::memory::StringView str) noexcept {
        canvas.background(Palette::black);
        canvas.foreground(Palette::white);
        canvas.text(0, 0, str);

        if (not _overlay.empty()) {
            const auto rows = 1 + (_overlay.size() / canvas.widthInGlyphs());
            const auto y = static_cast<kf::math::Pixels>(canvas.maxY() - rows * canvas.font().heightTotal());

            canvas.foreground(_overlay_color);
            canvas.rect(0, y, canvas.maxX(), canvas.maxY(), true);

            canvas.background(_overlay_color);
            canvas.foreground(Palette::black);
            canvas.text(overlay_text_padding, y, _overlay);
        }
    }

    void renderVirtualKeyboard(Canvas &canvas) noexcept {
        const auto longest_row = ui::VirtualKeyboard::rows[0].size();
        const auto key_width = canvas.width() / longest_row;
        const auto key_height = canvas.font().heightTotal();
        const auto keyboard_offset_y = canvas.maxY() - key_height * _virtual_keyboard.rowsTotal();
        const auto glyph_offset_x = (key_width - canvas.font().widthTotal()) / 2;

        canvas.text(0, 0, kf::memory::StaticString<32>::formatted("\xBC\xF0Text Input: %d / %d\x80\n", _virtual_keyboard.available(), _virtual_keyboard.text().size()).data());
        canvas.text(0, canvas.font().heightTotal(), _virtual_keyboard.text());

        canvas.background(Palette::dark_gray);
        canvas.foreground(Palette::dark_gray);
        canvas.rect(0, keyboard_offset_y, canvas.maxX(), canvas.maxY(), true);

        for (auto row = 0; row < _virtual_keyboard.rowsTotal(); row += 1) {
            const auto y = keyboard_offset_y + row * key_height;
            const auto cols = ui::VirtualKeyboard::rows[row].size();

            const auto x_offset = ((longest_row - cols) * key_width) / 2;

            for (auto col = 0; col < cols; col += 1) {
                const auto x = col * key_width + x_offset;

                if (row == _virtual_keyboard.cursorRow() and col == _virtual_keyboard.cursorCol()) {
                    canvas.foreground(Palette::dark_blue);
                    canvas.rect(x, y, x + key_width, y + key_height - 1, true);

                    canvas.background(Palette::dark_blue);
                    canvas.foreground(Palette::white);
                } else {
                    canvas.background(Palette::dark_gray);
                    canvas.foreground(Palette::black);
                }

                const auto &key = ui::VirtualKeyboard::keyAt(row, col);

                canvas.glyph(x + glyph_offset_x, y, key.isCommon() ? key.value(_virtual_keyboard.shifted()) : '?');
            }
        }
    }

    using This = GraphicsSystem<I>;

    DJC_IMPL_INITABLE(This, void(I &));
    void initImpl(DisplayDriver &display_driver) noexcept {
        _canvas = kf::some(Canvas{
            kf::image::DynamicImage<Pixel>{display_driver.image()},
            typename Canvas::State{
                .active_font = kf::someRef(kf::gfx::fonts::gyver_5x7_en),
                .auto_next_line = true,
            },
        });
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
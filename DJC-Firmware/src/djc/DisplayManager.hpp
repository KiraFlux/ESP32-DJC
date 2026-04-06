// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/gfx/Canvas.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/math/units.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/DeviceState.hpp"
#include "djc/prelude.hpp"
#include "djc/ui/UI.hpp"

namespace djc {

struct DisplayManager final : kf::mixin::NonCopyable, kf::mixin::Initable<DisplayManager, void> {

    explicit DisplayManager(DisplayDriver &display, const DeviceState &device_state) noexcept :
        _display{display}, _device_state{device_state} {}

private:
    DisplayDriver &_display;
    const DeviceState &_device_state;
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

        // Show mode indicator
        if (not _device_state.menu_navigation_enabled) {
            auto y = static_cast<kf::math::Pixels>(_canvas.maxY() - _canvas.glyphHeight());
            _canvas.text(0, y, "\xB2\xF0 Control Enabled");
        }

        _canvas.text(0, 0, str.data());
    }
};

}// namespace djc
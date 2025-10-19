#pragma once

#include <rs/ArrayString.hpp>

#include <kf/sys/abc/Component.hpp>


namespace kf::sys {

struct JoystickComponent final : kf::sys::Component {

    float x{0.0f};
    float y{0.0f};

    void display() override {
        constexpr auto text_offset = static_cast<gfx::Position>(3);
        constexpr auto format = "%+1.3f";

        const auto center_x = canvas.centerX();
        const auto center_y = canvas.centerY();

        const auto right_text_x = canvas.maxGlyphX() - text_offset;
        const auto text_offset_y = static_cast<gfx::Position>(center_y + text_offset);

        canvas.rect(0, 0, canvas.maxX(), canvas.maxY(), gfx::Canvas::Mode::FillBorder);

        canvas.line(
            center_x,
            center_y,
            static_cast<gfx::Position>(static_cast<float>(center_x) + x * static_cast<float>(center_x)),
            center_y);

        canvas.line(
            center_x,
            center_y,
            center_x,
            static_cast<gfx::Position>(static_cast<float>(center_y) - y * static_cast<float>(center_y)));

        canvas.setCursor(text_offset, text_offset);
        canvas.text(rs::formatted<8>(format, x).data());
        canvas.setCursor(static_cast<gfx::Position>(right_text_x), text_offset);
        canvas.text("X");

        canvas.setCursor(text_offset, text_offset_y);
        canvas.text(rs::formatted<8>(format, y).data());
        canvas.setCursor(static_cast<gfx::Position>(right_text_x), text_offset_y);
        canvas.text("Y");
    }
};

}// namespace djc

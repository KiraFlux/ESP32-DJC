#pragma once

#include <rs/ArrayString.hpp>

#include <kf/sys/abc/Element.hpp>


namespace kf::sys {

struct JoystickElement final : kf::sys::Element {

private:
    const float *x{nullptr};
    const float *y{nullptr};

public:
    void bindAxis(const float &axis_x, const float &axis_y) {
        x = &axis_x;
        y = &axis_y;
    }

    void display() override {
        constexpr auto text_offset = static_cast<kf::Position>(3);
        constexpr auto format = "%+1.3f";

        painter.rect(0, 0, painter.maxX(), painter.maxY(), kf::Painter::Mode::FillBorder);

        if (x == nullptr or y == nullptr) { return; }

        const auto center_x = painter.centerX();
        const auto center_y = painter.centerY();

        painter.line(
            center_x,
            center_y,
            static_cast<kf::Position>(static_cast<float>(center_x) + *x * static_cast<float>(center_x)),
            center_y);

        painter.line(
            center_x,
            center_y,
            center_x,
            static_cast<kf::Position>(static_cast<float>(center_y) - *y * static_cast<float>(center_y)));

        const auto right_text_x = painter.maxGlyphX() - text_offset;

        painter.setCursor(text_offset, text_offset);
        painter.text(rs::formatted<8>(format, *x).data());
        painter.setCursor(static_cast<kf::Position>(right_text_x), text_offset);
        painter.text("X");

        const auto text_offset_y = static_cast<kf::Position>(center_y + text_offset);

        painter.setCursor(text_offset, text_offset_y);
        painter.text(rs::formatted<8>(format, *y).data());
        painter.setCursor(static_cast<kf::Position>(right_text_x), text_offset_y);
        painter.text("Y");
    }
};

}// namespace djc

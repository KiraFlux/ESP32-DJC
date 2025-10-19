#pragma once

#include <kf/sys/abc/Element.hpp>


namespace kf::sys {

struct TextElement final : Element {

    const char *text;

    explicit TextElement(const char *text) :
        text{text} {}

    explicit TextElement() :
        text{nullptr} {}

    void display() override {
        painter.setCursor(0, 0);
        painter.text((text == nullptr) ? "null" : text);
    }

};

}

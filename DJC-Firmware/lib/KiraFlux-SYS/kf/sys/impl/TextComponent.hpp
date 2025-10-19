#pragma once

#include <kf/sys/abc/Component.hpp>


namespace kf::sys {

struct TextComponent final : Component {

    const char *text;

    explicit TextComponent(const char *text) :
        text{text} {}

    explicit TextComponent() :
        text{nullptr} {}

    void display() override {
        painter.setCursor(0, 0);
        painter.text((text == nullptr) ? "null" : text);
    }

};

}

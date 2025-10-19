#pragma once

#include <kf/sys/abc/Element.hpp>


namespace kf::sys {

struct FlagElement : Element {

public:
    const char *label{nullptr};
    const bool *flag{nullptr};

    void display() override {
        const bool lit = (flag != nullptr) and *flag;
        painter.setCursor(0, 0);
        painter.text_value_on = not lit;
        painter.text((label == nullptr) ? "null" : label);
    }
};

}// namespace kf::sys

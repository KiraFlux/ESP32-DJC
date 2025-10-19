#pragma once

#include <kf/sys/abc/Element.hpp>


namespace kf::sys {

struct FlagElement : Element {

private:
    const char *label{nullptr};
    bool value{false};

public:
    explicit FlagElement(const char *label, bool default_value = false) :
        label{label}, value{default_value} {}

    void display() override {
        painter.setCursor(0, 0);
        painter.text_value_on = not value;
        painter.text((label == nullptr) ? "null" : label);
    }

    inline void toggle() { value = not value; }

    inline void set(bool new_value) { value = new_value; }

    [[nodiscard]] inline bool get() const { return value; }

    operator bool() const { return get(); }
};

}// namespace kf::sys

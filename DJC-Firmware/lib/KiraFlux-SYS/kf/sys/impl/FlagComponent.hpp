#pragma once

#include <kf/sys/abc/Component.hpp>


namespace kf::sys {

struct FlagComponent : Component {

private:
    const char *label{nullptr};
    bool value{false};

public:
    explicit FlagComponent(const char *label, bool default_value = false) :
        label{label}, value{default_value} {}

    void display() override {
        canvas.setCursor(0, 0);
        canvas.text((label == nullptr) ? "null" : label, not value);
    }

    inline void toggle() { value = not value; }

    inline void set(bool new_value) { value = new_value; }

    [[nodiscard]] inline bool get() const { return value; }

    explicit operator bool() const { return get(); }
};

}// namespace kf::sys

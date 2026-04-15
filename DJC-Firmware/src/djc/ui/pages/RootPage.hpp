// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief Main menu page for ESP32-DJC
struct RootPage : UI::Page {
    static constexpr auto max_items{4};

    explicit constexpr RootPage() noexcept : Page{"Main"} {}

    void attach(UI::Page &page) noexcept {
        if (_items >= _layout.size()) { return; }

        _layout[_items] = &page.link();

        _items += 1;
        widgets({_layout.data(), _items});
    }

private:
    kf::memory::Array<UI::Widget *, max_items> _layout{};
    kf::usize _items{0};
};

}// namespace djc::ui::pages
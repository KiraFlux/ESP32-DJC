// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief Main menu page for ESP32-DJC
struct RootPage : UI::Page {
    kf::memory::Array<UI::Widget *, 2> widget_layout{{
        nullptr,// for MavLink link widget
        nullptr,// for Config link widget
    }};

    explicit constexpr RootPage() : Page{"ESP32-DJC"} {
        widgets({widget_layout.data(), widget_layout.size()});
    }
};

}// namespace djc
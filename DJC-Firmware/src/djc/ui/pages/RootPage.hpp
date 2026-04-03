// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief Main menu page for ESP32-DJC
struct RootPage : UI::Page {
    kf::memory::Array<UI::Widget *, 3> widget_layout{{
        nullptr,// for MavLinkControl link widget
        nullptr,// for RawControl link widget
        nullptr,// for Config link widget
    }};

    explicit constexpr RootPage() : Page{"ESP32-DJC"} {
        widgets({widget_layout.data(), widget_layout.size()});
    }
};

}// namespace djc
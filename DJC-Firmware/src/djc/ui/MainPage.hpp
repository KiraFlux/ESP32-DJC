// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/mixin/Singleton.hpp>

#include "djc/ui/UI.hpp"

namespace djc {

/// @brief Main menu page for ESP32-DJC
struct MainPage : UI::Page, kf::mixin::Singleton<MainPage> {
    kf::memory::Array<UI::Widget *, 1> widget_layout{{
        nullptr,// for MavLink link widget
    }};

    explicit constexpr MainPage() : Page{"ESP32-DJC"} {
        widgets({widget_layout.data(), widget_layout.size()});
    }
};

}// namespace djc
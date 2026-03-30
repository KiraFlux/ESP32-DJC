// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {
    kf::memory::Array<UI::Widget *, 1> widget_layout{};

    explicit ConfigPage(UI::Page &root) noexcept :
        Page{"Config"},
        widget_layout{{
            &root.link(),

        }} {
        widgets({widget_layout.data(), widget_layout.size()});
    }
};

}// namespace djc::pages

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/ui/Event.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/UiTraits.hpp>
#include <kf/ui/render/ColoredTextRender.hpp>
#include <kf/ui/Color.hpp>

#include "djc/ui/widgets/PeerDisplay.hpp"

namespace djc::internal {

using UiBase = kf::ui::UI<kf::ui::UiTraits<
    kf::ui::render::ColoredTextRender<256>,// Render Engine: Buffered Colored Text UI render engine
    kf::ui::Event<6>                       // Event: 6-bit Event value encoding
    >>;

}

namespace djc::ui {

/// @brief KiraFlux-Toolkit UI expended specializalization for ESP32-DJC
struct UI : internal::UiBase {
    using internal::UiBase::UiBase;

    using Color = kf::ui::Color;

    using Widget = internal::UiBase::Widget;

    struct PeerDisplay : widgets::PeerDisplay<Traits> {
        using widgets::PeerDisplay<Traits>::PeerDisplay;
    };
};

}// namespace djc::ui
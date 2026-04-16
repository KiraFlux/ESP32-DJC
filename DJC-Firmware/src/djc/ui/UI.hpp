// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/ui/Event.hpp>
#include <kf/ui/UI.hpp>
#include <kf/ui/render/ColoredTextRender.hpp>

namespace djc::ui {

// KiraFlux-Toolkit UI specialization for ESP32-DJC
using UI = kf::ui::UI<
    kf::ui::render::ColoredTextRender<256>,// Render Engine: Buffered Colored Text UI render engine
    kf::ui::Event<6>                       // Event: 6-bit Event value encoding
    >;

}// namespace djc
#pragma once

#include <kf/UI.hpp>
#include <kf/ui/Event.hpp>
#include <kf/ui/TextBufferRender.hpp>

namespace djc {

// KiraFlux-Toolkit UI spec
using UI = kf::UI<
    // Render Engine: Buffered Text UI render Engine
    kf::ui::TextBufferRender<256>,
    // Event: 2 bits - Event::Type, 6 bits - Event::value
    kf::ui::Event<2, 6>>;
}// namespace djc
#pragma once

#include <kf/UI.hpp>
#include <kf/ui/Event.hpp>
#include <kf/ui/TextBufferRender.hpp>


namespace djc {

// KiraFlux-Toolkit UI specialization for ESP32-DJC
using UI = kf::UI<
    // Render Engine: Buffered Text UI render engine
    kf::ui::TextBufferRender<256>,
    // Event: 6-bit Event value encoding
    kf::ui::Event<6>>;

} // namespace djc
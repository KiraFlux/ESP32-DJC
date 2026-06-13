// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/ui/Color.hpp>
#include <kf/ui/Event.hpp>
#include <kf/ui/Placement.hpp>
#include <kf/ui/render/ColoredTextRender.hpp>

#include "kf-patch/UI.hpp" // patched version

#include "djc/ui/UiTraits.hpp"
#include "djc/ui/VirtualKeyboard.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::internal {

using UiBase = ::kf::ui::UI<::djc::ui::UiTraits<
    ::kf::ui::render::ColoredTextRender<256>,// Render Engine: Buffered Colored Text UI render engine
    ::kf::ui::Event<6>                       // Event: 6-bit Event value encoding
    >>;

}

namespace djc::ui {

/// @brief ESP32-DJC extended UI specializalization
/// @note djc::pages must use fields from this service (`UI::Color`, `UI::Widget`, etc.)
struct UI : internal::UiBase {

    explicit constexpr UI(Traits::RenderImpl &render_system, VirtualKeyboard &virtual_keyboard) noexcept :
        internal::UiBase{render_system}, _virtual_keyboard{virtual_keyboard} {}

    /// @brief UI Semantic Color
    using Color = kf::ui::Color;

    /// @brief UI Widget value Placement
    using Placement = kf::ui::Placement;

    /// @brief UI Widget Base
    using Widget = internal::UiBase::Widget;

    /// @brief Transport Peer display Widget
    struct PeerDisplay : widgets::PeerDisplay<Traits> {
        using widgets::PeerDisplay<Traits>::PeerDisplay;
    };

    /// @brief Text input area via Virtual Keyboard
    struct TextInput : widgets::TextInput<Traits> {
        using widgets::TextInput<Traits>::TextInput;
    };

    /// @brief Create text input widget with virtual keyboard binding
    [[nodiscard]] TextInput createTextInput(kf::Slice<char> source = {}) noexcept {
        return TextInput{
            _virtual_keyboard,
            source,
        };
    }

private:
    VirtualKeyboard &_virtual_keyboard;
};

}// namespace djc::ui
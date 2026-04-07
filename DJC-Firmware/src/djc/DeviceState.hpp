// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/aliases.hpp>
#include <kf/mixin/NonCopyable.hpp>

namespace djc {

struct DeviceState final : kf::mixin::NonCopyable {
    enum class Mode : kf::u8 {
        UiNavigation,
        Control,
        KeyboardInput,
    };

    [[nodiscard]] bool uiNavigationEnabled() const noexcept { return mode == Mode::UiNavigation; }

    [[nodiscard]] bool controlEnabled() const noexcept { return mode == Mode::Control; }

    [[nodiscard]] bool keyboardInputEnabled() const noexcept { return mode == Mode::KeyboardInput; }

    // fields

    Mode mode;
};

}// namespace djc
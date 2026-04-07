// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/mixin/NonCopyable.hpp>

namespace djc {

struct DeviceState final : kf::mixin::NonCopyable {
    enum class Mode {
        UiNavigation,
        Control,
    };

    Mode mode;

    [[nodiscard]] bool controlEnabled() const noexcept { return mode == Mode::Control; }    
};

}// namespace djc
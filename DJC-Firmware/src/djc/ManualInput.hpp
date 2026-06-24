// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/primitives.hpp>

namespace djc {

/// @brief Normalized manual control input from joysticks
/// @note
/// Holds four stick channels (left/right, X/Y) in a fixed‑point format.
/// The values are sent to the peer by the active protocol.
struct ManualInput final {
    using Unit = kf::i16;

    /// @brief Scale factor - normalized float [‑1, +1] is multiplied by this value
    /// @note Mavlink manual control compatable scaling
    static constexpr Unit scale_factor{1000};

    Unit left_x, left_y, right_x, right_y;///< Joystick axes

    /// @brief Convert a normalized float value to the internal unit representation
    [[nodiscard]] static constexpr Unit fromNormalized(kf::f32 value) noexcept {
        return static_cast<Unit>(value * scale_factor);
    }
};

}// namespace djc
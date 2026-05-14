// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/transport/EspNowTransport.hpp"
#include "djc/transport/Kind.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

/// @brief Registry providing access to available transport implementations.
struct TransportRegistry {

    /// @brief Returns a transport instance by kind (ignored, always returns ESP-NOW).
    /// @param kind Requested transport kind (not used, kept for future extension).
    /// @return Reference to the ESP‑NOW transport.
    [[nodiscard]] Transport &get(Kind kind) noexcept {
        (void) kind;
        return _espnow_transport;
    }

    /// @brief Direct access to the ESP‑NOW transport instance.
    [[nodiscard]] EspNowTransport &espnow() noexcept { return _espnow_transport; }

private:
    EspNowTransport _espnow_transport{};
};

}// namespace djc::transport
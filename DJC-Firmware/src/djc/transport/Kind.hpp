// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace djc::transport {

/// @brief Identifies the underlying transport technology of a peer address.
enum class Kind {

    /// @brief ESP‑NOW protocol (built‑in WiFi, peer‑to‑peer frames).
    EspNow,
};

}// namespace djc::transport
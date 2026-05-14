// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/transport/EspNowTransport.hpp"
#include "djc/transport/Kind.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

struct TransportRegistry {

    [[nodiscard]] Transport &get(Kind kind) noexcept {
        (void) kind;
        return _espnow_transport;
    }

    [[nodiscard]] EspNowTransport &espnow() noexcept { return _espnow_transport; }

private:
    EspNowTransport _espnow_transport{};
};

}// namespace djc::transport
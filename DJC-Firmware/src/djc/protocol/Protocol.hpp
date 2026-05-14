// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/aliases.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::protocol {

/// @brief Abstract protocol for sending control data and processing incoming packets
struct Protocol : kf::mixin::NonCopyable {

    /// @brief Called periodically when the protocol is active and the link is connected
    /// @note
    /// The implementation must serialise the current stick values and send them through the transport.
    /// Periodic tasks such as heartbeat or keep‑alive messages are also handled here, using `now` to maintain internal timers.
    virtual void poll(kf::math::Milliseconds now, const ManualInput &input, transport::TransportLink &transport_link) noexcept = 0;

    /// @brief Process an incoming data buffer from the connected peer
    /// @note
    /// The implementation must parse the data according to its protocol and notify the appropriate callback
    /// This method is called from the transport callback; it must be fast and never block.
    virtual void receive(kf::memory::Slice<const kf::u8> buffer) noexcept = 0;
};

}// namespace djc::protocol
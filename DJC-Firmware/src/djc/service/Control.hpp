// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/service/Service.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::service {

/// @brief Thin bridge between transport and protocol layers, enabling manual control transmission.
/// @note
/// Stores the current joystick input and a flag that enables/disables output.
/// When enabled and the transport is connected, `pollImpl()` forwards the input to the active protocol via `ProtocolLink::poll()`.
struct Control final : Service<Control> {

    explicit Control(transport::TransportLink &transport_link, protocol::ProtocolLink &protocol_link) noexcept :
        _transport_link{transport_link}, _protocol_link{protocol_link} {}

    /// @brief Returns the current manual input values.
    [[nodiscard]] const ManualInput &input() const noexcept { return _manual_input; }

    /// @brief Updates the manual input to be transmitted.
    void input(const ManualInput &new_input) noexcept { _manual_input = new_input; }

    /// @brief Checks whether control output is enabled.
    [[nodiscard]] bool enabled() const noexcept { return _enabled; }

    /// @brief Enables or disables control output.
    void enabled(bool is_enabled) noexcept { _enabled = is_enabled; }

private:
    transport::TransportLink &_transport_link;
    protocol::ProtocolLink &_protocol_link;
    ManualInput _manual_input{};
    bool _enabled{false};

    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_enabled and _transport_link.connected()) {
            _protocol_link.poll(now, _manual_input, _transport_link);
        }
    }
};

}// namespace djc::service
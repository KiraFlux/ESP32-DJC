// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc {

struct Control final : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Control> {

    explicit Control(transport::TransportLink &transport_link, protocol::ProtocolLink &protocol_link) noexcept :
        _transport_link{transport_link}, _protocol_link{protocol_link} {}

    [[nodiscard]] const ManualInput &input() const noexcept { return _manual_input; }

    void input(const ManualInput &new_input) noexcept { _manual_input = new_input; }

    [[nodiscard]] bool enabled() const noexcept { return _enabled; }

    void enabled(bool is_enabled) noexcept { _enabled = is_enabled; }

private:
    transport::TransportLink &_transport_link;
    protocol::ProtocolLink &_protocol_link;
    ManualInput _manual_input{};
    bool _enabled{false};

    // impl
    KF_IMPL_TIMED_POLLABLE(Control);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_enabled and _transport_link.connected()) {
            _protocol_link.poll(now, _manual_input, _transport_link);
        }
    }
};

}// namespace djc
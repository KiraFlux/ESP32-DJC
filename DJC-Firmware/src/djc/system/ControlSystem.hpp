// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/ManualInput.hpp"
#include "djc/Periphery.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/service/Control.hpp"
#include "djc/system/System.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::system {

/// @brief System managing manual control output.
/// @note Owns Control service. Depends Periphery (joysticks), TransportLink and ProtocolLink.
struct ControlSystem : System<ControlSystem> {

    explicit ControlSystem(Periphery &periphery, transport::TransportLink &transport_link, protocol::ProtocolLink &protocol_link) noexcept :
        _periphery{periphery}, _control{transport_link, protocol_link} {}

    /// @brief Get mutable access to the control service
    service::Control &service() noexcept {
        return _control;
    }

    /// @brief Get readonly access to the control service
    constexpr const service::Control &service() const noexcept {
        return _control;
    }

private:
    Periphery &_periphery;
    service::Control _control;

    KF_IMPL_INITABLE(ControlSystem, bool);
    bool initImpl() noexcept {
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(ControlSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_control.enabled()) {
            using I = ManualInput;

            _control.input(I{
                .left_x = I::fromNormalized(_periphery.left_joystick.axis_x.read()),
                .left_y = I::fromNormalized(_periphery.left_joystick.axis_y.read()),
                .right_x = I::fromNormalized(_periphery.right_joystick.axis_x.read()),
                .right_y = I::fromNormalized(_periphery.right_joystick.axis_y.read()),
            });
        }

        _control.poll(now);
    }
};

}// namespace djc::system
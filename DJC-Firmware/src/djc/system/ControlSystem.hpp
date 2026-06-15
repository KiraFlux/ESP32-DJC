// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/ManualInput.hpp"
#include "djc/Periphery.hpp"
#include "djc/mixin/ServiceOwner.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/service/ControlService.hpp"
#include "djc/system/System.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::system {

/// @brief System managing manual control output
/// @note Owns Control service. Depends Periphery (joysticks), TransportLink and ProtocolLink.
struct ControlSystem :

    System<ControlSystem, void()>,
    mixin::ServiceOwner<service::ControlService>

{
    explicit ControlSystem(Periphery &periphery, transport::TransportLink &transport_link, protocol::ProtocolLink &protocol_link) noexcept :
        mixin::ServiceOwner<service::ControlService>{service::ControlService{transport_link, protocol_link}}, _periphery{periphery} {}

private:
    Periphery &_periphery;

    DJC_IMPL_INITABLE(ControlSystem, void());
    void initImpl() noexcept {}

    KF_IMPL_TIMED_POLLABLE(ControlSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (this->service().enabled()) {
            using I = ManualInput;

            this->service().input(I{
                .left_x = I::fromNormalized(_periphery.left_joystick.axis_x.read()),
                .left_y = I::fromNormalized(_periphery.left_joystick.axis_y.read()),
                .right_x = I::fromNormalized(_periphery.right_joystick.axis_x.read()),
                .right_y = I::fromNormalized(_periphery.right_joystick.axis_y.read()),
            });
        }

        this->service().poll(now);
    }
};

}// namespace djc::system
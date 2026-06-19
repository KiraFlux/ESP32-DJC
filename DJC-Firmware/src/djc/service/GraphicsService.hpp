// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/drivers/display/DisplayDriver.hpp>

#include "djc/service/Service.hpp"

namespace djc::service {

/// @brief Service that manages display sending
template<typename I> struct GraphicsService : Service<GraphicsService<I>> {
    KF_CHECK_IMPL(I, ::kf::drivers::display::DisplayDriverTag);

    explicit constexpr GraphicsService(I &display_driver) noexcept :
        _display_driver{display_driver} {}

    void requestSend() noexcept {
        _send_requested = true;
    }

private:
    I &_display_driver;
    bool _send_requested{false};

    KF_IMPL_TIMED_POLLABLE(GraphicsService<I>);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_send_requested) {
            _send_requested = false;
            (void) _display_driver.send();
        }
    }
};

}// namespace djc::service
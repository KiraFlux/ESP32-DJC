// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/mixin/ServiceOwner.hpp"
#include "djc/service/DisplayManager.hpp"
#include "djc/system/System.hpp"
#include "djc/ui/VirtualKeyboard.hpp"

namespace djc::system {

/// @brief System managing display output, canvas, and virtual keyboard.
template<typename I> struct GraphicsSystem :

    System<GraphicsSystem<I>, void()>,
    mixin::ServiceOwner<service::DisplayManager<I>>

{
    using DisplayManagerImpl = service::DisplayManager<I>;

    explicit GraphicsSystem(I &display_driver, const ui::VirtualKeyboard &virtual_keyboard) noexcept :
        mixin::ServiceOwner<DisplayManagerImpl>{DisplayManagerImpl{display_driver, virtual_keyboard}} {}

private:
    using This = GraphicsSystem<I>;

    DJC_IMPL_INITABLE(This, void());
    void initImpl() noexcept {
        this->service().init();
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
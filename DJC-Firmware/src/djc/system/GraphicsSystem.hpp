// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/service/DisplayManager.hpp"
#include "djc/system/System.hpp"
#include "djc/ui/VirtualKeyboard.hpp"

namespace djc::system {

/// @brief System managing display output, canvas, and virtual keyboard.
template<typename I> struct GraphicsSystem : System<GraphicsSystem<I>> {
    using DisplayManagerImpl = service::DisplayManager<I>;

    explicit GraphicsSystem(I &display_driver, const ui::VirtualKeyboard &virtual_keyboard) noexcept :
        _display_manager{_display_driver, virtual_keyboard} {}

    /// @brief Get mutable access to display service
    DisplayManagerImpl &service() noexcept {
        return _display_manager;
    }

    /// @brief Get readonly access to display service
    constexpr const DisplayManagerImpl &service() const noexcept {
        return _display_manager;
    }

private:
    DisplayManagerImpl _display_manager;

    using This = GraphicsSystem<I>;

    KF_IMPL_INITABLE(This, bool);
    bool initImpl() noexcept {
        _display_manager.init();
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _display_manager.poll(now);
    }
};

}// namespace djc::system
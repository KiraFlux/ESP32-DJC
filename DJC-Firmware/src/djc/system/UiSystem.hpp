// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <initializer_list>

#include <kf/Logger.hpp>

#include "djc/Config.hpp"
#include "djc/mixin/ServiceOwner.hpp"
#include "djc/system/System.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/VirtualKeyboard.hpp"
#include "djc/ui/pages/RootPage.hpp"

namespace djc::system {

/// @brief System that owns UI components: renderer, virtual keyboard, UI instance and root page
/// @note Initialises the UI with the root page and triggers an initial update event. Polls the UI on every main loop iteration
struct UiSystem :

    System<UiSystem, void(std::initializer_list<ui::UI::Page *>)>,
    mixin::ServiceOwner<djc::ui::UI>

{
    using Renderer = ui::UI::Traits::RenderImpl;

    explicit UiSystem(const Config &config) noexcept :
        mixin::ServiceOwner<djc::ui::UI>{djc::ui::UI{_renderer, _virtual_keyboard}},
        _renderer{config.render_system} {}

    /// @brief Get mutable access to renderer component
    Renderer &renderer() noexcept {
        return _renderer;
    }

    /// @brief Get readonly access to renderer component
    constexpr const Renderer &renderer() const noexcept {
        return _renderer;
    }

    /// @brief Get mutable access to virtual keyboard component
    ui::VirtualKeyboard &virtualKeyboard() noexcept {
        return _virtual_keyboard;
    }

    /// @brief Get readonly access to virtual keyboard component
    constexpr const ui::VirtualKeyboard &virtualKeyboard() const noexcept {
        return _virtual_keyboard;
    }

    /// @brief Get mutable access to ui service
    ui::pages::RootPage &rootPage() noexcept {
        return _root_page;
    }

    /// @brief Get readonly access to ui service
    constexpr const ui::pages::RootPage &rootPage() const noexcept {
        return _root_page;
    }

private:
    static constexpr auto logger{kf::Logger::create("UiSystem")};

    Renderer _renderer;
    ui::VirtualKeyboard _virtual_keyboard{};
    ui::pages::RootPage _root_page{this->service()};

    DJC_IMPL_INITABLE(UiSystem, void(std::initializer_list<ui::UI::Page *>));
    void initImpl(std::initializer_list<ui::UI::Page *> pages) noexcept {
        for (auto page : pages) {
            _root_page.attach(*page);
        }
        
        this->service().activePage(_root_page);
        this->service().addEvent(ui::UI::Traits::EventImpl::update());
    }

    KF_IMPL_TIMED_POLLABLE(UiSystem);
    void poll(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
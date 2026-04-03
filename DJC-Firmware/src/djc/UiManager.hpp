// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/ui/UI.hpp"
#include "djc/ui/pages/RootPage.hpp"

namespace djc {

struct UiManager final : kf::mixin::NonCopyable, kf::mixin::Initable<UiManager, void>, kf::mixin::TimedPollable<UiManager> {

    void addEvent(ui::UI::Event event) noexcept { ui.addEvent(event); }

    [[nodiscard]] ui::pages::RootPage &root() noexcept { return _root; }

private:
    ui::pages::RootPage _root{};
    ui::UI &ui{ui::UI::instance()};

    // impl

    KF_IMPL_INITABLE(UiManager, void);
    void initImpl() noexcept {
        ui.bindPage(_root);
        ui.addEvent(ui::UI::Event::update());
    }

    KF_IMPL_TIMED_POLLABLE(UiManager);
    void pollImpl(kf::math::Milliseconds now) noexcept { ui.poll(now); }
};

}// namespace djc
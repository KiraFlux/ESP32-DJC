// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Storage.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/Control.hpp"
#include "djc/DeviceConfig.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

namespace djc {

struct UiManager final : kf::mixin::NonCopyable, kf::mixin::Initable<UiManager, void>, kf::mixin::TimedPollable<UiManager> {

    void addEvent(ui::UI::Event event) noexcept { ui.addEvent(event); }

    explicit UiManager(Control &control, kf::memory::Storage<DeviceConfig> &storage) noexcept :
        mav_link_control{root, control}, config{root, storage} {}

private:
    // pages

    ui::pages::RootPage root{};
    ui::pages::MavLinkControlPage mav_link_control;
    ui::pages::ConfigPage config;

    ui::UI &ui{ui::UI::instance()};

    // impl

    KF_IMPL_INITABLE(UiManager, void);
    void initImpl() noexcept {
        // apply page links
        // WARNING: before adding another link check RootPage::widget_layout LENGTH
        root.widget_layout[0] = &mav_link_control.link();
        root.widget_layout[1] = &config.link();

        // prepare UI
        ui.bindPage(root);
        ui.addEvent(ui::UI::Event::update());
    }

    KF_IMPL_TIMED_POLLABLE(UiManager);
    void pollImpl(kf::math::Milliseconds now) noexcept { ui.poll(now); }
};

}// namespace djc
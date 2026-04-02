// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/ui/UI.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

namespace djc {

struct UiManager final : kf::mixin::NonCopyable, kf::mixin::Initable<UiManager, void> {
    ui::pages::RootPage root{};

    // User pages

    ui::pages::MavLinkControlPage mav_link_control{root};
    ui::pages::ConfigPage config{root};

private:
    KF_IMPL_INITABLE(UiManager, void);
    void initImpl() noexcept {
        // apply page links
        // WARNING: before adding another link check RootPage::widget_layout LENGTH
        root.widget_layout[0] = &mav_link_control.link();
        root.widget_layout[1] = &config.link();

        // prepare UI
        auto &ui = ui::UI::instance();
        ui.bindPage(root);
        ui.addEvent(ui::UI::Event::update());
    }
};

}// namespace djc
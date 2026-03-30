#pragma once

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/mixin/Initable.hpp>
#include <kf/mixin/Singleton.hpp>

#include "djc/ui/UI.hpp"
#include "djc/ui/pages/ConfigPage.hpp"
#include "djc/ui/pages/MavLinkControlPage.hpp"
#include "djc/ui/pages/RootPage.hpp"

namespace djc::ui::pages {

struct PageManager final : kf::mixin::Singleton<PageManager>, kf::mixin::Initable<PageManager, void> {
    RootPage root{};

    // User pages

    MavLinkControlPage mav_link_control{root};
    ConfigPage config{root};

private:
    // impl
    KF_IMPL_INITABLE(PageManager, void);
    void initImpl() noexcept {
        // apply page links
        // WARNING: before adding another link check RootPage::widget_layout LENGTH
        root.widget_layout[0] = &mav_link_control.link();
        root.widget_layout[1] = &config.link();

        // prepare UI
        auto &ui = UI::instance();
        ui.bindPage(root);
        ui.addEvent(UI::Event::update());
    }
};

}// namespace djc::pages

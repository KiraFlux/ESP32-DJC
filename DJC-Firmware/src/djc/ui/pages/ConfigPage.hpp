// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ConfigManager.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(UI::Page &root, ConfigManager &storage) noexcept :
        Page{"Config"},
        widget_layout{{
            &root.link(),
            &_save_storage,
            &_load_storage,
            &_reset_storage,
        }} {
        widgets({widget_layout.data(), widget_layout.size()});

        _save_storage.callback([&storage]() {
            storage.save();
        });

        _load_storage.callback([&storage]() {
            storage.load();
        });

        _reset_storage.callback([&storage]() {
            storage.reset();
        });
    }

private:
    // widgets
    UI::Button _save_storage{"Save"};
    UI::Button _load_storage{"Load"};
    UI::Button _reset_storage{"Reset"};
    kf::memory::Array<UI::Widget *, 4> widget_layout;
};

}// namespace djc::ui::pages

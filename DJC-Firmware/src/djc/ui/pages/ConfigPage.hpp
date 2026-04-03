// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>
#include <kf/memory/Storage.hpp>

#include "djc/DeviceConfig.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(UI::Page &root, kf::memory::Storage<DeviceConfig> &storage) noexcept :
        Page{"Config"},
        widget_layout{{
            &root.link(),
            &_save_storage,
            &_load_storage,
            &_erase_storage,
        }}, _storage{storage} {
        widgets({widget_layout.data(), widget_layout.size()});

        _save_storage.callback([&storage](){
            storage.save();
        });

        _load_storage.callback([&storage](){
            storage.load();
        });

        _erase_storage.callback([&storage](){
            storage.erase();
        });
    }

private:
    kf::memory::Storage<DeviceConfig> &_storage;
    
    // widgets
    UI::Button _save_storage{"Save"};
    UI::Button _load_storage{"Load"};
    UI::Button _erase_storage{"Erase"};
    kf::memory::Array<UI::Widget *, 4> widget_layout;
};

}// namespace djc::ui::pages

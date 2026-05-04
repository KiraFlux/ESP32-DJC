// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ConfigManager.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/TextInput.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(UI::Page &root) noexcept :
        Page{"Config"},
        _layout{{
            &root.link(),
            &_device_name_input,
            &_labeled_default_protocol_mode_selector,
            &_save_storage,
            &_load_storage,
            &_reset_storage,
        }} {
        widgets({_layout.data(), _layout.size()});

        _device_name_input.source({storage.config().device_name.data(), storage.config().device_name.size()});

        _save_storage.callback([]() {
            storage.save();
        });

        _load_storage.callback([]() {
            storage.load();
        });

        _reset_storage.callback([]() {
            storage.reset();
        });

        _default_protocol_mode_selector.callback([](Mode mode) {
            storage.config().init_protocol_mode = mode;
        });
    }

private:
    using Mode = protocol::ProtocolRegistry::Mode;
    using ProtocolModeSelector = UI::ComboBox<Mode>;

    inline static auto &storage{djc::ConfigManager::instance()};

    // widgets
    widgets::TextInput _device_name_input{};
    UI::Button _save_storage{"Save"};
    UI::Button _load_storage{"Load"};
    UI::Button _reset_storage{"Reset (RAM cache)"};

    kf::memory::Array<ProtocolModeSelector::Item, 2> _control_mode_options{{
        {protocol::ProtocolRegistry::stringFromMode(Mode::Mavlink), Mode::Mavlink},
        {protocol::ProtocolRegistry::stringFromMode(Mode::Raw), Mode::Raw},
    }};

    ProtocolModeSelector::Config _control_mode_config{
        .items = {_control_mode_options.data(), _control_mode_options.size()},
    };

    // TODO: set init value from storage
    ProtocolModeSelector _default_protocol_mode_selector{_control_mode_config};

    UI::Labeled _labeled_default_protocol_mode_selector{"Init Protocol", _default_protocol_mode_selector};

    // layout
    kf::memory::Array<UI::Widget *, 6> _layout;
};

}// namespace djc::ui::pages
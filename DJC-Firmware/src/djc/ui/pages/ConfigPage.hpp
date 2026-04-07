// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/memory/Array.hpp>

#include "djc/ConfigManager.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/TextInput.hpp"
#include "djc/Keyboard.hpp"

namespace djc::ui::pages {

struct ConfigPage : UI::Page {

    explicit ConfigPage(UI::Page &root, Keyboard &keyboard) noexcept :
        Page{"Config"},
        _test_input{keyboard},
        _layout{{
            &root.link(),
            &_test_input,
            &_init_mode_selector_label,
            &_save_storage,
            &_load_storage,
            &_reset_storage,
        }} {
        widgets({_layout.data(), _layout.size()});

        static auto &storage = djc::ConfigManager::instance();

        static kf::memory::Array<char, 100> f{"12345"};
        _test_input.source({f.data(), f.size()});

        _save_storage.callback([]() {
            storage.save();
        });

        _load_storage.callback([]() {
            storage.load();
        });

        _reset_storage.callback([]() {
            storage.reset();
        });

        _init_mode_selector.callback([](Control::Mode init_mode) {
            storage.config().control.init_mode = init_mode;
        });
    }

private:
    using ControlModeSelectWidget = UI::ComboBox<Control::Mode>;

    // widgets
    widgets::TextInput _test_input;
    UI::Button _save_storage{"Save"};
    UI::Button _load_storage{"Load"};
    UI::Button _reset_storage{"Reset"};

    kf::memory::Array<ControlModeSelectWidget::Item, 2> _control_mode_options{{
        {Control::stringFromMode(Control::Mode::MavLink), Control::Mode::MavLink},
        {Control::stringFromMode(Control::Mode::Raw), Control::Mode::Raw},
    }};

    ControlModeSelectWidget::Config _control_mode_config{
        .items = {_control_mode_options.data(), _control_mode_options.size()},
    };

    // TODO: set init value from storage
    ControlModeSelectWidget _init_mode_selector{_control_mode_config};

    UI::Labeled _init_mode_selector_label{"Init Control", _init_mode_selector};

    // layout
    kf::memory::Array<UI::Widget *, 6> _layout;
};

}// namespace djc::ui::pages

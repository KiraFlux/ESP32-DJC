// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/algorithm.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/input/VirtualKeyboard.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct TextInput final : UI::Widget {

    void source(kf::memory::Slice<char> new_source) noexcept { _text_source = new_source; }

    bool available() const noexcept { return nullptr != _text_source.data(); }

    void doRender(UI::RenderImpl &render) const noexcept override {
        if (not available()) {
            render.value(kf::memory::StringView{"not available"});
            return;
        }

        const kf::memory::StringView s{_text_source.data(), _text_source.size()};
        render.value(s.sub(0, s.find('\0').value()));
    }

    bool onClick() noexcept override {
        if (not available()) { return false; }

        auto &virtual_keyboard = input::VirtualKeyboard::instance();

        if (virtual_keyboard.active()) {
            virtual_keyboard.click();
        } else {
            virtual_keyboard.begin(_text_source);
        }

        return true;
    }

    bool onEventValue(UI::Event::Value event_value) noexcept {
        auto &virtual_keyboard = input::VirtualKeyboard::instance();

        if (virtual_keyboard.active()) {
            virtual_keyboard.move(static_cast<input::VirtualKeyboard::Direction>(event_value));
            return true;
        }

        return false;
    }

private:
    kf::memory::Slice<char> _text_source{};
};

}// namespace djc::ui::widgets
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

    constexpr TextInput() noexcept : _text_source{} {}

    explicit constexpr TextInput(kf::memory::Slice<char> source) noexcept : _text_source{source} {}

    void source(kf::memory::Slice<char> new_source) noexcept { _text_source = new_source; }

    bool available() const noexcept { return nullptr != _text_source.data(); }

    void doRender(UI::RenderImpl &render) const noexcept override {
        if (not available()) {
            render.value(kf::memory::StringView{"not available"});
            return;
        }

        const kf::memory::StringView s{_text_source.data(), _text_source.size()};
        const auto end_index = s.find('\0');
        render.value(end_index.hasValue() ? s.sub(0, end_index.value()) : s);
    }

    bool onClick() noexcept override {
        if (not available()) { return false; }

        if (virtual_keyboard.active()) {
            virtual_keyboard.click();
        } else {
            virtual_keyboard.begin(_text_source);
        }

        return true;
    }

    bool onEventValue(UI::Event::Value event_value) noexcept {
        if (virtual_keyboard.active()) {
            virtual_keyboard.move(static_cast<input::VirtualKeyboard::Direction>(event_value));
            return true;
        }

        return false;
    }

private:
    inline static auto &virtual_keyboard{input::VirtualKeyboard::instance()};

    kf::memory::Slice<char> _text_source;
};

}// namespace djc::ui::widgets
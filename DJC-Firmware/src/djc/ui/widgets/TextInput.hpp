// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

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
        render.value(kf::memory::StringView{"\xFC'"});
        render.value(string());
        render.value(kf::memory::StringView{"'\x80"});
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

    [[nodiscard]] kf::memory::StringView string() const noexcept {
        if (available()) {
            const kf::memory::StringView s{_text_source.data(), _text_source.size()};
            return s.sub(0, s.find('\0').valueOr(s.size()));
        } else {
            return kf::memory::StringView{"not available"};
        }
    }
};

}// namespace djc::ui::widgets
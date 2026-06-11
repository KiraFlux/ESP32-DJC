// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/algorithm.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/input/VirtualKeyboard.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct TextInput final : UI::Widget {

    TextInput() noexcept : _text_source{} {
        initStyle();
    }

    explicit TextInput(kf::Slice<char> source) noexcept : _text_source{source} {
        initStyle();
    }

    void source(kf::Slice<char> new_source) noexcept {
        _text_source = new_source;
    }

    bool available() const noexcept {
        return nullptr != _text_source.data();
    }

    void doRender(UI::Traits::RenderImpl &render) const noexcept override {
        render.value(string());
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

    bool onEventValue(UI::Traits::EventImpl::Value event_value) noexcept {
        if (not virtual_keyboard.active()) {
            return false;
        }

        switch (event_value) {
            case 0: virtual_keyboard.moveCursorRow(-1); break;
            case 1: virtual_keyboard.moveCursorRow(+1); break;
            case 2: virtual_keyboard.moveCursorCol(-1); break;
            case 3: virtual_keyboard.moveCursorCol(+1); break;
        }

        return true;
    }

private:
    void initStyle() {
        foreground(UI::Color::Info);
    }

    inline static auto &virtual_keyboard{input::VirtualKeyboard::instance()};

    kf::Slice<char> _text_source;

    [[nodiscard]] kf::memory::StringView string() const noexcept {
        if (available()) {
            const kf::memory::StringView s{_text_source.data(), _text_source.size()};
            return s.sub(0, s.find('\0').unwrapOr(s.size()));
        } else {
            return kf::memory::StringView{"not available"};
        }
    }
};

}// namespace djc::ui::widgets
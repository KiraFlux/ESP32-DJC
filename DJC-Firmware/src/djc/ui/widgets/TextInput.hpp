// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/ui/Color.hpp>
#include <kf/ui/Style.hpp>

#include "djc/ui/VirtualKeyboard.hpp"

namespace djc::ui::widgets {

template<typename U> struct TextInput :

    U::Widget

{
    explicit TextInput(VirtualKeyboard &virtual_keyboard, kf::Slice<char> source, kf::ui::Style style = kf::ui::Style::defaults()) noexcept :
        U::Widget{style}, _virtual_keyboard{virtual_keyboard}, _text_source{source} {
        this->foreground(kf::ui::Color::Info);
    }

    void source(kf::Slice<char> new_source) noexcept {
        _text_source = new_source;
    }

    bool available() const noexcept {
        return nullptr != _text_source.data();
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.value('\"');
        render.value(string());
        render.value('\"');
    }

    bool onClick() noexcept override {
        if (not available()) { return false; }

        if (_virtual_keyboard.active()) {
            _virtual_keyboard.click();
        } else {
            _virtual_keyboard.begin(_text_source);
        }

        return true;
    }

    bool onEventValue(typename U::EventImpl::Value event_value) noexcept {
        if (not _virtual_keyboard.active()) {
            return false;
        }

        switch (event_value) {
            case 0: _virtual_keyboard.moveCursorRow(-1); break;
            case 1: _virtual_keyboard.moveCursorRow(+1); break;
            case 2: _virtual_keyboard.moveCursorCol(-1); break;
            case 3: _virtual_keyboard.moveCursorCol(+1); break;
        }

        return true;
    }

private:
    VirtualKeyboard &_virtual_keyboard;
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
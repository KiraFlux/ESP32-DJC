// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/algorithm.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Keyboard.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct TextInput final : UI::Widget {

    explicit constexpr TextInput(Keyboard &keyboard) noexcept : _keyboard{keyboard} {}

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

        if (_keyboard.active()) {
            _keyboard.click();
        } else {
            _keyboard.begin(_text_source);
        }

        return true;
    }

    bool onEventValue(UI::Event::Value event_value) noexcept {
        if (_keyboard.active()) {
            _keyboard.move(static_cast<Keyboard::Direction>(event_value));
            return true;
        }

        return false;
    }

private:
    Keyboard &_keyboard;
    kf::memory::Slice<char> _text_source{};
};

}// namespace djc::ui::widgets
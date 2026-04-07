// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/algorithm.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/DeviceState.hpp"

namespace djc {

struct Keyboard final : kf::mixin::NonCopyable {

    enum class Direction : kf::u8 {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3,
    };

    struct Key {
        char value;
    };

    static constexpr auto keys_total{10};

    static constexpr kf::memory::Array<Key, keys_total> keys{{
        // row: 1
        {'Q'},
        {'W'},
        {'E'},
        {'R'},
        {'T'},
        {'Y'},
        {'U'},
        {'I'},
        {'O'},
        {'P'},

    }};

    explicit constexpr Keyboard(DeviceState &device_state) noexcept : _device_state{device_state} {}

    bool active() const noexcept { return _device_state.keyboardInputEnabled(); }

    kf::memory::StringView text() const noexcept { return {_text_source.data(), _text_source.size()}; }

    kf::usize available() const noexcept {
        if (_text_cursor < _text_source.size()) {
            return _text_source.size() - _text_cursor;
        }

        return 0;
    }

    kf::i8 selectedButtonIndex() const noexcept { return _selected_button_index; }

    void begin(kf::memory::Slice<char> text_source) noexcept {
        _device_state.mode = DeviceState::Mode::KeyboardInput;

        _text_source = text_source;
        _text_cursor = text().find('\0').value();
    }

    void quit() noexcept {
        _device_state.mode = DeviceState::Mode::UiNavigation;

        _text_source = {};
    }

    void click() noexcept {
        if (available() == 0) { return; }

        _text_source[_text_cursor] = keys[_selected_button_index].value;

        _text_cursor += 1;
    }

    void move(Direction direction) noexcept {
        switch (direction) {
            case Direction::Down:
            case Direction::Up:
                _text_cursor = kf::max(0u, _text_cursor - 1);
                _text_source[_text_cursor] = '\0';
                return;

            case Direction::Left:
                moveCursor(-1);
                return;

            case Direction::Right:
                moveCursor(+1);
                return;
        }
    }

private:
    DeviceState &_device_state;
    kf::memory::Slice<char> _text_source{};
    kf::usize _text_cursor{};
    kf::i8 _selected_button_index{0};

    void moveCursor(kf::i8 delta) noexcept {
        using T = decltype(_selected_button_index);
        _selected_button_index = kf::clamp<T>(static_cast<T>(_selected_button_index + delta), 0, keys.size() - 1);
    }
};

}// namespace djc

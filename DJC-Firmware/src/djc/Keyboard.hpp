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

    static constexpr auto keys_total{38};

    static constexpr kf::memory::Array<Key, keys_total> keys{{
        // row 0: 10 keys
        {'1'},
        {'2'},
        {'3'},
        {'4'},
        {'5'},
        {'6'},
        {'7'},
        {'8'},
        {'9'},
        {'0'},

        // row 1: 10 keys
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

        // row 2: 9 keys
        {'A'},
        {'S'},
        {'D'},
        {'F'},
        {'G'},
        {'H'},
        {'J'},
        {'K'},
        {'L'},

        // row 3: (7 + 2 wide) keys
        {'^'},// mock SHIFT
        {'Z'},
        {'X'},
        {'C'},
        {'V'},
        {'B'},
        {'N'},
        {'M'},
        {'<'},// mock BACKSPACE
    }};

    static constexpr kf::memory::Array<kf::u8, 4> keys_in_row{{10, 10, 9, 9}};

    explicit constexpr Keyboard(DeviceState &device_state) noexcept : _device_state{device_state} {}

    kf::u8 rowsTotal() const noexcept { return keys_in_row.size(); }

    kf::u8 row() const noexcept { return _selected_key_row; }

    kf::u8 colsTotal() const noexcept { return keys_in_row[_selected_key_row]; }

    kf::u8 col() const noexcept { return _selected_key_col; }

    static kf::u8 selectedIndex(kf::i8 rr, kf::i8 cc) noexcept {
        auto ret = 0;

        for (auto r = 0; r < rr; r += 1) {
            ret += keys_in_row[r];
        }

        return ret + cc;
    }

    kf::u8 selectedIndex() const noexcept {
        return selectedIndex(row(), col());
    }

    [[nodiscard]] bool active() const noexcept { return _device_state.keyboardInputEnabled(); }

    [[nodiscard]] kf::memory::StringView text() const noexcept { return {_text_source.data(), _text_source.size()}; }

    [[nodiscard]] kf::usize available() const noexcept {
        if (_text_cursor < _text_source.size()) {
            return _text_source.size() - _text_cursor;
        } else {
            return 0;
        }
    }

    void begin(kf::memory::Slice<char> text_source) noexcept {
        _device_state.mode = DeviceState::Mode::KeyboardInput;

        _text_source = text_source;
        _text_cursor = text().find('\0').value();
    }

    void click() noexcept {
        if (available() == 0) { return; }

        _text_source[_text_cursor] = keys[selectedIndex()].value;
        _text_cursor += 1;
        _text_source[_text_cursor] = '\0';
    }

    void move(Direction direction) noexcept {
        switch (direction) {
            case Direction::Down:
                // backspace functional here is a temp. solution
                _text_cursor = kf::max(0, _text_cursor - 1);
                _text_source[_text_cursor] = '\0';
                return;

            case Direction::Up:
                moveCursorRow(-1);
                return;

            case Direction::Left:
                moveCursorCol(-1);
                return;

            case Direction::Right:
                moveCursorCol(+1);
                return;
        }
    }

private:
    DeviceState &_device_state;
    kf::memory::Slice<char> _text_source{};
    kf::isize _text_cursor{};
    kf::i8 _selected_key_row{0}, _selected_key_col{0};

    void moveCursorRow(kf::i8 delta) noexcept {
        _selected_key_row = (_selected_key_row + delta + rowsTotal()) % rowsTotal();
    }

    void moveCursorCol(kf::i8 delta) noexcept {
        _selected_key_col = (_selected_key_col + delta + colsTotal()) % colsTotal();
    }
};

}// namespace djc

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/algorithm.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/Singleton.hpp>

namespace djc::input {

struct Key {

    enum class Kind : kf::u8 {
        Common,
        Shift,
        Backspace,
        Space,
        Enter,
    };

    constexpr Key(char normal, char shift) noexcept : kind{Kind::Common}, normal_value{normal}, shift_value{shift} {}

    constexpr Key(Kind k, char value) noexcept : kind{k}, normal_value{value}, shift_value{0} {}

    const Kind kind;
    const char normal_value, shift_value;

    constexpr char value(bool shifted = false) const noexcept {
        return shifted ? shift_value : normal_value;
    }
};

struct VirtualKeyboard final : kf::mixin::Singleton<VirtualKeyboard> {

    enum class Direction : kf::u8 {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3,
    };

    enum class State : kf::u8 {
        Normal,
        ShiftOnce,
        ShiftAll,
    };

    template<kf::usize N> using KeyRow = kf::memory::Array<Key, N>;

    static constexpr KeyRow<14> row_0{{
        {'`', '~'},
        {'1', '!'},
        {'2', '@'},
        {'3', '#'},
        {'4', '$'},
        {'5', '%'},
        {'6', '^'},
        {'7', '&'},
        {'8', '*'},
        {'9', '('},
        {'0', ')'},
        {'-', '_'},
        {'=', '+'},
        {Key::Kind::Backspace, 0},        
    }};

    static constexpr KeyRow<13> row_1{{
        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'},
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'},
        {'[', '{'},
        {']', '}'},
        {'\\', '|'},
    }};

    static constexpr KeyRow<12> row_2{{
        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},
        {';', ':'},
        {'\'', '"'},
        {Key::Kind::Enter, '\n'},
    }};

    static constexpr KeyRow<10> row_3{{
        {Key::Kind::Shift, 0},
        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},
        {'m', 'M'},
        {',', '<'},
        {'.', '>'},
    }};

    static constexpr KeyRow<1> row_4{{
        {Key::Kind::Space, ' '},
    }};

    static constexpr kf::memory::Array<kf::memory::Slice<const Key>, 5> rows{{
        {row_0.data(), row_0.size()},
        {row_1.data(), row_1.size()},
        {row_2.data(), row_2.size()},
        {row_3.data(), row_3.size()},
        {row_4.data(), row_4.size()},
    }};

    [[nodiscard]] kf::u8 rowsTotal() const noexcept { return rows.size(); }

    [[nodiscard]] kf::u8 colsTotal() const noexcept { return rows[_cursor_row].size(); }

    [[nodiscard]] kf::u8 cursorRow() const noexcept { return _cursor_row; }

    [[nodiscard]] kf::u8 cursorCol() const noexcept { return _cursor_row_index; }

    [[nodiscard]] bool shifted() const noexcept { return _state != State::Normal; }

    [[nodiscard]] bool active() const noexcept { return _active; }

    [[nodiscard]] kf::memory::StringView text() const noexcept { return {_text_source.data(), _text_source.size()}; }

    [[nodiscard]] static const Key &keyAt(kf::i8 row, kf::i8 col) noexcept {
        return rows[row][col];
    }

    [[nodiscard]] kf::usize available() const noexcept {
        if (_text_cursor < _text_source.size()) {
            return _text_source.size() - _text_cursor;
        } else {
            return 0;
        }
    }

    void begin(kf::memory::Slice<char> text_source) noexcept {
        _active = true;

        _text_source = text_source;
        _text_cursor = text().find('\0').value();
    }

    void quit() noexcept {
        _active = false;
    }

    void click() noexcept {
        if (available() == 0) { return; }

        const auto &key = rows[_cursor_row][_cursor_row_index];

        switch (key.kind) {
            case Key::Kind::Space:
            case Key::Kind::Enter:
            case Key::Kind::Common: {
                _text_source[_text_cursor] = key.value(shifted());
                _text_cursor += 1;
                _text_source[_text_cursor] = '\0';

                if (State::ShiftOnce == _state) { _state = State::Normal; }
            }
                return;

            case Key::Kind::Shift: {
                _state = evolvedState(_state);
            }
                return;

            case Key::Kind::Backspace: {
                _text_cursor = kf::max(0, _text_cursor - 1);
                _text_source[_text_cursor] = '\0';
            }
                return;
        }
    }

    void move(Direction direction) noexcept {
        switch (direction) {
            case Direction::Down:
                moveCursorRow(+1);
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
    kf::memory::Slice<char> _text_source{};
    kf::isize _text_cursor{};
    kf::i8 _cursor_row{0}, _cursor_row_index{0};
    bool _active{false};
    State _state{State::Normal};

    void moveCursorRow(kf::i8 delta) noexcept {
        _cursor_row = (_cursor_row + delta + rowsTotal()) % rowsTotal();
        _cursor_row_index = kf::clamp<kf::i8>(_cursor_row_index, 0, colsTotal() - 1);
    }

    void moveCursorCol(kf::i8 delta) noexcept {
        _cursor_row_index = (_cursor_row_index + delta + colsTotal()) % colsTotal();
    }

    static State evolvedState(State state) noexcept {
        switch (state) {
            case State::Normal:
                return State::ShiftOnce;
            case State::ShiftOnce:
                return State::ShiftAll;
            case State::ShiftAll:
            default:
                return State::Normal;
        }
    }
};

}// namespace djc::input

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Function.hpp>
#include <kf/algorithm.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/Singleton.hpp>

namespace djc::input {

struct VirtualKeyboard final : kf::mixin::Singleton<VirtualKeyboard> {

    enum class Direction : kf::u8 {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3,
    };

    struct Key {

        enum class Kind {
            Common,
            Shift,
            Backspace,
            Space,
            Enter,
        };

        static constexpr Key createCommon(char value) noexcept {
            return Key{
                .kind = Kind::Common,
                .value = value,
            };
        }

        Kind kind;
        char value;
    };

    template<kf::usize N> using KeyRow = kf::memory::Array<Key, N>;
    using KeyView = kf::memory::Slice<const Key>;

    static constexpr KeyRow<10> row_0{{
        {.kind = Key::Kind::Common, .value = '1'},
        {.kind = Key::Kind::Common, .value = '2'},
        {.kind = Key::Kind::Common, .value = '3'},
        {.kind = Key::Kind::Common, .value = '4'},
        {.kind = Key::Kind::Common, .value = '5'},
        {.kind = Key::Kind::Common, .value = '6'},
        {.kind = Key::Kind::Common, .value = '7'},
        {.kind = Key::Kind::Common, .value = '8'},
        {.kind = Key::Kind::Common, .value = '9'},
        {.kind = Key::Kind::Common, .value = '0'},
    }};

    static constexpr KeyRow<10> row_1{{
        {.kind = Key::Kind::Common, .value = 'Q'},
        {.kind = Key::Kind::Common, .value = 'W'},
        {.kind = Key::Kind::Common, .value = 'E'},
        {.kind = Key::Kind::Common, .value = 'R'},
        {.kind = Key::Kind::Common, .value = 'T'},
        {.kind = Key::Kind::Common, .value = 'Y'},
        {.kind = Key::Kind::Common, .value = 'U'},
        {.kind = Key::Kind::Common, .value = 'I'},
        {.kind = Key::Kind::Common, .value = 'O'},
        {.kind = Key::Kind::Common, .value = 'P'},
    }};

    static constexpr KeyRow<9> row_2{{
        {.kind = Key::Kind::Common, .value = 'A'},
        {.kind = Key::Kind::Common, .value = 'S'},
        {.kind = Key::Kind::Common, .value = 'D'},
        {.kind = Key::Kind::Common, .value = 'F'},
        {.kind = Key::Kind::Common, .value = 'G'},
        {.kind = Key::Kind::Common, .value = 'H'},
        {.kind = Key::Kind::Common, .value = 'J'},
        {.kind = Key::Kind::Common, .value = 'K'},
        {.kind = Key::Kind::Common, .value = 'L'},
    }};

    static constexpr KeyRow<9> row_3{{
        {.kind = Key::Kind::Shift, .value = 0},
        {.kind = Key::Kind::Common, .value = 'Z'},
        {.kind = Key::Kind::Common, .value = 'X'},
        {.kind = Key::Kind::Common, .value = 'C'},
        {.kind = Key::Kind::Common, .value = 'V'},
        {.kind = Key::Kind::Common, .value = 'B'},
        {.kind = Key::Kind::Common, .value = 'N'},
        {.kind = Key::Kind::Common, .value = 'M'},
        {.kind = Key::Kind::Backspace, .value = 0},
    }};

    static constexpr KeyRow<4> row_4{{
        {.kind = Key::Kind::Common, .value = ','},
        {.kind = Key::Kind::Space, .value = 0},
        {.kind = Key::Kind::Common, .value = '.'},
        {.kind = Key::Kind::Enter, .value = 0},
    }};

    static constexpr kf::memory::Array<KeyView, 5> rows{{
        {row_0.data(), row_0.size()},
        {row_1.data(), row_1.size()},
        {row_2.data(), row_2.size()},
        {row_3.data(), row_3.size()},
        {row_4.data(), row_4.size()},
    }};

    kf::u8 rowsTotal() const noexcept { return rows.size(); }

    kf::u8 row() const noexcept { return _selected_key_row; }

    kf::u8 colsTotal() const noexcept { return rows[_selected_key_row].size(); }

    kf::u8 col() const noexcept { return _selected_key_col; }

    [[nodiscard]] bool active() const noexcept { return _active; }

    [[nodiscard]] kf::memory::StringView text() const noexcept { return {_text_source.data(), _text_source.size()}; }

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

        const auto &selected_key = rows[_selected_key_row][_selected_key_col];

        switch (selected_key.kind) {
            case Key::Kind::Common: {
                _text_source[_text_cursor] = selected_key.value;
                _text_cursor += 1;
                _text_source[_text_cursor] = '\0';
            }
                return;

            case Key::Kind::Shift: {
            }
                return;

            case Key::Kind::Backspace: {
                _text_cursor = kf::max(0, _text_cursor - 1);
                _text_source[_text_cursor] = '\0';
            }
                return;

            case Key::Kind::Space: {
            }
                return;

            case Key::Kind::Enter: {
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
    kf::i8 _selected_key_row{0}, _selected_key_col{0};
    bool _active{false};

    void moveCursorRow(kf::i8 delta) noexcept {
        _selected_key_row = (_selected_key_row + delta + rowsTotal()) % rowsTotal();
    }

    void moveCursorCol(kf::i8 delta) noexcept {
        _selected_key_col = (_selected_key_col + delta + colsTotal()) % colsTotal();
    }
};

}// namespace djc::input

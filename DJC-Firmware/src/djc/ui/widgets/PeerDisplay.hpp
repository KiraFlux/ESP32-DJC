// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/PeerScanner.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct PeerDisplay final : UI::Widget {

    enum class State : char {
        Cleared = '\xF8',
        Stable = '\xFC',
        PreCleared = '\xFC',
    };

    static constexpr kf::math::Milliseconds pre_cleared_highligt_timespan{2000};

    void transportLink(transport::TransportLink &new_transport_link) noexcept { _transport_link = &new_transport_link; }

    void update(const kf::Option<PeerScanner::Entry> &entry_option, kf::math::Milliseconds deadline_time) noexcept {
        _entry_option = entry_option;

        if (_entry_option.hasValue()) {
            _state = (deadline_time < _entry_option.value().last_seen + pre_cleared_highligt_timespan) ? State::Stable : State::PreCleared;
        } else {
            _state = State::Cleared;
        }
    }

    void clear() noexcept {
        _entry_option = {};
    }

    void doRender(UI::RenderImpl &render) const noexcept override {
        constexpr auto empty_string = "    -    -    ";
        const auto content = (_entry_option.hasValue()) ? _entry_option.value().address.toString().data() : empty_string;

        render.beginBlock();
        render.value(kf::memory::ArrayString<64>::formatted("%c%s\x80", static_cast<char>(_state), content).view());
        render.endBlock();
    }

    bool onClick() noexcept override {
        if (not _entry_option.hasValue()) { return false; }

        if (_transport_link != nullptr and _transport_link->connect(_entry_option.value().address)) {
            clear();
        }

        return true;
    }

private:
    transport::TransportLink *_transport_link{nullptr};
    kf::Option<PeerScanner::Entry> _entry_option{};
    State _state{State::Cleared};
};

}// namespace djc::ui::widgets
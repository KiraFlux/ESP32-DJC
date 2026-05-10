// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/Callbacked.hpp>

#include "djc/PeerScanner.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct PeerDisplay final : UI::Widget, kf::mixin::Callbacked<const transport::PeerAddress &> {

    enum class State : char {
        Cleared = 0,
        Stable = '\xFC',
        PreCleared = '\xF9',
    };

    static constexpr auto extreme_age_factor{0.75f};

    void update(const kf::Option<PeerScanner::Entry> &entry, kf::Option<kf::memory::StringView> entry_name, kf::math::Milliseconds now, kf::math::Milliseconds max_life_time) noexcept {
        _entry = entry;
        _entry_name = entry_name;

        if (_entry.hasValue()) {
            const auto age = now - _entry.value().last_seen;
            _state = (age >= max_life_time * extreme_age_factor) ? State::PreCleared : State::Stable;
        } else {
            _state = State::Cleared;
        }
    }

    void doRender(UI::RenderImpl &render) const noexcept override {
        render.beginBlock();

        if (_entry.hasValue()) {
            const auto content = _entry_name.hasValue() ? _entry_name.value().data() : _entry.value().address.toString().data();
            render.value(kf::memory::ArrayString<64>::formatted("%c%s\x80", static_cast<char>(_state), content).view());
        }

        render.endBlock();
    }

    bool onClick() noexcept override {
        if (_entry.hasValue()) {
            this->invoke(_entry.value().address);
        }
        return _entry.hasValue();
    }

private:
    kf::Option<PeerScanner::Entry> _entry{};
    kf::Option<kf::memory::StringView> _entry_name{};
    State _state{State::Cleared};
};

}// namespace djc::ui::widgets
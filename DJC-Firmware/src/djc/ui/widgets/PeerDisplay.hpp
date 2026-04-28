// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::widgets {

struct PeerDisplay final : UI::Widget {

    enum class State : kf::u8 {
        Cleared,
        NewConnection,
        Stable,
        PreCleared,
    };

    static constexpr kf::math::Milliseconds clear_timeout{8000}, new_highlight_timespan{clear_timeout - 600}, pre_cleared_highligt_timespan{2000};

    void transportLink(transport::TransportLink &new_transport_link) noexcept { _transport_link = &new_transport_link; }

    const kf::Option<transport::PeerAddress> &address() const noexcept { return _address; }

    void update(const transport::PeerAddress &address, kf::math::Milliseconds now) noexcept {
        _mac_clear_timer.start(now);
        _address.value(address);
    }

    void checkForClear(kf::math::Milliseconds now) noexcept {
        if (_mac_clear_timer.expired(now)) {
            _address = {};
        }

        if (_address.hasValue()) {

            if (_mac_clear_timer.remaining(now) > new_highlight_timespan) {
                _state = State::NewConnection;
            } else if (_mac_clear_timer.remaining(now) < pre_cleared_highligt_timespan) {
                _state = State::PreCleared;
            } else {
                _state = State::Stable;
            }

        } else {
            _state = State::Cleared;
        }
    }

    void doRender(UI::RenderImpl &render) const noexcept override {
        render.beginBlock();

        if (_state == State::NewConnection) {
            render.value(kf::memory::StringView{"\xFC"});
        } else if (_state == State::PreCleared) {
            render.value(kf::memory::StringView{"\xF9"});
        }

        if (_address.hasValue()) {
            render.value(_address.value().toString().view());
        } else {
            render.value(kf::memory::StringView{"\xF8    -    -    "});
        }

        if (_state != State::Stable) {
            render.value(kf::memory::StringView{"\x80"});
        }

        render.endBlock();
    }

    bool onClick() noexcept override {
        if (not _address.hasValue()) { return false; }

        if (_transport_link != nullptr) {
            (void) _transport_link->connect(_address.value());
            _address = {};
        }

        return true;
    }

private:
    transport::TransportLink *_transport_link{nullptr};
    kf::Option<transport::PeerAddress> _address{};
    kf::math::Timer _mac_clear_timer{clear_timeout};
    State _state{State::Cleared};
};

}// namespace djc::ui::widgets
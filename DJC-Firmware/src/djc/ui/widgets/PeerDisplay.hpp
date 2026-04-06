// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Option.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Control.hpp"
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

    void control(Control &control) noexcept { _control = &control; }

    bool matches(const Control::EspNow::Mac &mac) const noexcept {
        return (not _mac_option.hasValue()) or (_mac_option.hasValue() and _mac_option.value() == mac);
    }

    void update(const Control::EspNow::Mac &mac, kf::math::Milliseconds now) noexcept {
        _mac_clear_timer.start(now);
        _mac_option.value(mac);
    }

    void checkForClear(kf::math::Milliseconds now) noexcept {
        if (_mac_clear_timer.expired(now)) {
            _mac_option = {};
        }

        if (_mac_option.hasValue()) {

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
            render.value(kf::memory::StringView{"\xFA"});
        } else if (_state == State::PreCleared) {
            render.value(kf::memory::StringView{"\xF9"});
        }

        if (_mac_option.hasValue()) {
            const auto &mac = _mac_option.value();

            if (_control != nullptr and _control->activePeer().hasValue() and _control->activePeer().value() == mac) {
                render.arrow();
            }

            render.value(Control::EspNow::stringFromMac(mac).view());
        } else {
            render.value(kf::memory::StringView{"\xF8    -    -    "});
        }

        if (_state != State::Stable) {
            render.value(kf::memory::StringView{"\x80"});
        }

        render.endBlock();
    }

    bool onClick() noexcept override {
        if (not _mac_option.hasValue()) { return false; }

        if (_control != nullptr) {
            // TODO: add to favorites?
            _control->activePeer(_mac_option.value());
        }

        return true;
    }

private:
    Control *_control{nullptr};
    kf::Option<Control::EspNow::Mac> _mac_option{};
    kf::math::Timer _mac_clear_timer{clear_timeout};
    State _state{State::Cleared};
};

}// namespace djc::ui::widgets
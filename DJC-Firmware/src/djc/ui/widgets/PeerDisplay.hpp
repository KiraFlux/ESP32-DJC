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

    constexpr explicit PeerDisplay(Control &control) noexcept : _control{control} {}

    void update(const Control::EspNow::Mac &mac, kf::math::Milliseconds now) noexcept {
        _mac_clear_timer.start(now);
        _mac_option.value(mac);
    }

    void checkForClear(kf::math::Milliseconds now) noexcept {
        if (_mac_clear_timer.expired(now)) {
            _mac_option = {};
        }
    }

    void doRender(UI::RenderImpl &render) const noexcept override {
        if (_mac_option.hasValue()) {
            const auto &mac = _mac_option.value();

            if (_control.activePeer().hasValue() and _control.activePeer().value() == mac) {
                render.arrow();
            }

            render.value(Control::EspNow::stringFromMac(mac).view());
        } else {
            render.value(kf::memory::StringView{"- - -"});
        }
    }

    bool onClick() noexcept override {
        if (not _mac_option.hasValue()) { return false; }

        _control.activePeer(_mac_option.value());
        return true;
    }

    bool onEventValue(UI::Event::Value event_value) noexcept override {
        return false;
    }

private:
    Control &_control;
    kf::Option<Control::EspNow::Mac> _mac_option{};
    kf::math::Timer _mac_clear_timer{static_cast<kf::math::Milliseconds>(5000)};
};

}// namespace djc::ui::widgets

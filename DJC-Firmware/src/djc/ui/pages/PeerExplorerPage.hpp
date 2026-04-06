// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h> // for millis

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/Control.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"

namespace djc::ui::pages {

struct PeerExplorerPage : UI::Page {

    static constexpr auto max_peer_display{8};
    static constexpr auto peer_display_start_index{1};

    static constexpr kf::math::Milliseconds redraw_period{500};


    explicit constexpr PeerExplorerPage(UI::Page &root, Control &control) noexcept :
        Page{"Peer Explorer"},
        _control{control},
        _layout{{
            &root.link(),
        }}

    {
        for (auto i = 0; i < _peer_displays.size(); i += 1) {
            _peer_displays[i].control(_control);
            _layout[i + peer_display_start_index] = &_peer_displays[i];
        }

        widgets({_layout.data(), _layout.size()});

        _redraw_timer.start(millis());
    }

    void onEntry() noexcept override {
        logger.debug("entry");

        _control.onReceiveFromUnknown([this](const Control::EspNow::Mac &mac, kf::memory::Slice<const kf::u8> data) {
            logger.info(
                kf::memory::ArrayString<64>::formatted(
                    "Got %d bytes from %s",
                    data.size(),
                    Control::EspNow::stringFromMac(mac).data()));

            getMatched(mac).update(mac, millis());
        });
    }

    void onExit() noexcept override {
        logger.debug("exit");

        _control.onReceiveFromUnknown(Control::ReceiveFromUnknownCallback{nullptr});
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        for (auto &_peer_display: _peer_displays) {
            _peer_display.checkForClear(now);
        }

        if (_redraw_timer.expired(now)) {
            _redraw_timer.start(now);
            UI::instance().addEvent(UI::Event::update());
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("PeerExplorerPage")};

    Control &_control;
    kf::math::Timer _redraw_timer{redraw_period};

    // widgets
    kf::memory::Array<widgets::PeerDisplay, max_peer_display> _peer_displays{};

    // layout
    kf::memory::Array<UI::Widget *, (peer_display_start_index + max_peer_display)> _layout;

    widgets::PeerDisplay &getMatched(const Control::EspNow::Mac &mac) noexcept {
        for (auto &_peer_display: _peer_displays) {
            if (_peer_display.matches(mac)) {
                return _peer_display;
            }
        }

        return _peer_displays[0];
    }
};

}// namespace djc::ui::pages
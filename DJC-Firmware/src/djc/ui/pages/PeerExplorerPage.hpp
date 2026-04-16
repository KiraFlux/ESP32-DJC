// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Arduino.h>// for millis

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/Control.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"
#include "djc/prelude.hpp"

namespace djc::ui::pages {

struct PeerExplorerPage : UI::Page {

    static constexpr auto max_peer_display{8};
    static constexpr auto peer_display_start_index{3};
    static constexpr kf::math::Milliseconds redraw_period{500};

    explicit constexpr PeerExplorerPage(UI::Page &root, Control &control) noexcept :
        Page{"Peer Explorer"},
        _control{control},
        _layout{{
            &root.link(),
            &_connection_button,
            &_available_label,
        }}

    {
        for (auto i = 0; i < _peer_displays.size(); i += 1) {
            _peer_displays[i].control(_control);
            _layout[i + peer_display_start_index] = &_peer_displays[i];
        }

        _connection_button.callback([this]() {
            if (_control.activeMac().hasValue()) {
                _control.disconnect();
            }
        });

        widgets({_layout.data(), _layout.size()});

        _redraw_timer.start(millis());
    }

    void onEntry() noexcept override {
        _control.onReceiveFromUnknown([this](const EspNow::Mac &mac, kf::memory::Slice<const kf::u8> data) {
            logger.debug(
                kf::memory::ArrayString<64>::formatted(
                    "Got %d bytes from %s",
                    data.size(),
                    EspNow::stringFromMac(mac).data()));

            getMatched(mac).update(mac, millis());
        });
    }

    void onExit() noexcept override {
        _control.onReceiveFromUnknown(Control::ReceiveFromUnknownCallback{nullptr});
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        for (auto &_peer_display: _peer_displays) {
            _peer_display.checkForClear(now);
        }

        if (_redraw_timer.expired(now)) {
            _redraw_timer.start(now);

            if (_control.activeMac().hasValue()) {
                (void) _connection_button_label.format(
                    "\xFC""OK: %s\x80",
                    EspNow::stringFromMac(_control.activeMac().value()).data());
                _connection_button.label(_connection_button_label.view());
            } else {
                _connection_button.label("\xF9""Disconnected\x80");
            }

            (void) _available_label_value.format(" Available: %d", countAvailablePeers());
            _available_label.value(_available_label_value.view());

            UI::instance().addEvent(UI::Event::update());
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("PeerExplorerPage")};

    Control &_control;
    kf::math::Timer _redraw_timer{redraw_period};
    kf::memory::ArrayString<16> _available_label_value{""};
    kf::memory::ArrayString<64> _connection_button_label{};

    // widgets
    UI::Button _connection_button{""};
    UI::Display<kf::memory::StringView> _available_label{_available_label_value.view()};
    kf::memory::Array<widgets::PeerDisplay, max_peer_display> _peer_displays{};

    // layout
    kf::memory::Array<UI::Widget *, (peer_display_start_index + max_peer_display)> _layout;

    widgets::PeerDisplay &getMatched(const EspNow::Mac &mac) noexcept {
        for (auto &_peer_display: _peer_displays) {
            if (not _peer_display.mac().hasValue()) { return _peer_display; }
            if (_peer_display.mac().value() == mac) { return _peer_display; }
        }

        return _peer_displays[0];
    }

    int countAvailablePeers() const noexcept {
        int available = 0;
        for (auto &_peer_display: _peer_displays) {
            available += int(_peer_display.mac().hasValue());
        }
        return available;
    }
};

}// namespace djc::ui::pages
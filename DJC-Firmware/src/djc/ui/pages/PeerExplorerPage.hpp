// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/PeerScanner.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"

namespace djc::ui::pages {

struct PeerExplorerPage : UI::Page {

    static constexpr auto peer_display_start_index{3};
    static constexpr kf::math::Milliseconds redraw_period{1'000};

    explicit constexpr PeerExplorerPage(UI::Page &root, PeerScanner &peer_scanner, transport::TransportLink &transport_link) noexcept :
        Page{"Peer Explorer"},
        _peer_scanner{peer_scanner},
        _transport_link{transport_link},
        _layout{{
            &root.link(),
            &_connection_button,
            &_available_label,
        }}

    {
        for (auto i = 0; i < _peer_displays.size(); i += 1) {
            _peer_displays[i].transportLink(_transport_link);
            _layout[i + peer_display_start_index] = &_peer_displays[i];
        }

        _connection_button.callback([this]() {
            if (_transport_link.connected()) {
                _transport_link.disconnect();
            }
        });

        widgets({_layout.data(), _layout.size()});

        _redraw_timer.start(0);// enable timer
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        if (_redraw_timer.expired(now)) {
            _redraw_timer.start(now);

            if (_transport_link.activePeerAddress().hasValue()) {
                (void) _connection_button_label.format("\xFC%s\x80", _transport_link.activePeerAddress().value().toString().data());
                _connection_button.label(_connection_button_label.view());
            } else {
                _connection_button.label("\xF9"
                                         "Disconnected\x80");
            }

            const auto available_peers = _peer_scanner.peers();

            (void) _available_label_value.format(" Available: %d", available_peers.size());
            _available_label.value(_available_label_value.view());

            for (auto i = 0u; i < _peer_displays.size(); i += 1) {
                if (i < available_peers.size()) {
                    _peer_displays[i].update(available_peers[i], now, _peer_scanner.config().entry_max_life_time);
                } else {
                    _peer_displays[i].clear();
                }
            }

            UI::instance().addEvent(UI::Event::update());
        }
    }

private:
    PeerScanner &_peer_scanner;
    transport::TransportLink &_transport_link;
    kf::math::Timer _redraw_timer{redraw_period};
    kf::memory::ArrayString<64> _available_label_value{}, _connection_button_label{};

    // widgets
    UI::Button _connection_button{""};
    UI::Display<kf::memory::StringView> _available_label{_available_label_value.view()};
    kf::memory::Array<widgets::PeerDisplay, PeerScanner::max_entries> _peer_displays{};

    // layout
    kf::memory::Array<UI::Widget *, (peer_display_start_index + PeerScanner::max_entries)> _layout;
};

}// namespace djc::ui::pages
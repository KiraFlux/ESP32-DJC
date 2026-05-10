// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/PeerScanner.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerDetailPage.hpp"
#include "djc/ui/widgets/PeerDisplay.hpp"

namespace djc::ui::pages {

struct PeerExplorerPage : UI::Page {

    explicit PeerExplorerPage(
        UI::Page &root,
        transport::TransportLink &transport_link,
        PeerScanner &peer_scanner,
        PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{"Peer Explorer"},
        _transport_link{transport_link},
        _peer_scanner{peer_scanner},
        _peer_favorites_registry{peer_favorites_registry},
        _layout{{
            &root.link(),
            &_primary_connection_status_button,
            &_available_label,
        }} {
        for (auto i = 0u; i < _peer_displays.size(); i += 1) {
            _peer_displays[i].callback([this](const transport::PeerAddress &address) -> void {
                _peer_detail_page.bindPeer(address);
                UI::instance().bindPage(_peer_detail_page);
            });

            _layout[i + peer_display_start_index] = &_peer_displays[i];
        }

        _primary_connection_status_button.callback([this]() {
            if (_transport_link.connected()) {
                _transport_link.disconnect();
            }
        });

        widgets(layout(0));

        _redraw_timer.start(0);// enable timer
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        if (not _redraw_timer.expired(now)) { return; }
        _redraw_timer.start(now);

        if (_transport_link.activePeerAddress().hasValue()) {
            (void) _connection_button_buffer.format("\xFC%s\x80", _transport_link.activePeerAddress().value().toString().data());
            _primary_connection_status_button.label(_connection_button_buffer.view());
        } else {
            _primary_connection_status_button.label(
                "\xF9"
                "Disconnected\x80");
        }

        const auto available_peers = _peer_scanner.peers();
        (void) _available_label_buffer.format(" Available: %d", available_peers.size());
        _available_label.value(_available_label_buffer.view());

        for (auto i = 0u; i < available_peers.size(); i += 1) {
            _peer_displays[i].state(createPeerDisplayState(available_peers[i], now));
        }

        widgets(layout(available_peers.size()));
        UI::instance().addEvent(UI::Event::update());
    }

private:
    static constexpr auto peer_display_start_index{3u};

    transport::TransportLink &_transport_link;
    PeerScanner &_peer_scanner;
    PeerFavoritesRegistry &_peer_favorites_registry;
    kf::math::Timer _redraw_timer{static_cast<kf::math::Milliseconds>(500)};

    kf::memory::ArrayString<64> _available_label_buffer{}, _connection_button_buffer{};

    UI::Button _primary_connection_status_button{{}};
    UI::Display<kf::memory::StringView> _available_label{_available_label_buffer.view()};
    kf::memory::Array<widgets::PeerDisplay, PeerScanner::max_entries> _peer_displays{};

    kf::memory::Array<UI::Widget *, (peer_display_start_index + PeerScanner::max_entries)> _layout;

    // child pages
    PeerDetailPage _peer_detail_page{*this, _transport_link, _peer_favorites_registry};

    kf::memory::Slice<UI::Widget *> layout(kf::usize displayed_peers) noexcept {
        return kf::memory::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(peer_display_start_index + displayed_peers);
    }

    kf::Option<widgets::PeerDisplay::State> createPeerDisplayState(const kf::Option<PeerScanner::Entry> &entry, kf::math::Milliseconds now) const noexcept {
        using P = widgets::PeerDisplay;
        constexpr auto extreme_age_factor{0.75f};

        const auto map_record = [](const kf::Option<PeerFavoritesRegistry::Entry> &record) -> kf::Option<kf::memory::StringView> {
            if (record.hasValue()) {
                const auto &name = record.value().name;
                return {{name.data(), name.size()}};
            } else {
                return {};
            }
        };

        if (entry.hasValue()) {
            const auto age = now - entry.value().last_seen;
            const auto extreme_age = _peer_scanner.config().entry_max_life_time * extreme_age_factor;
            return {P::State{
                .address = entry.value().address,
                .name = map_record(_peer_favorites_registry.get(entry.value().address)),
                .label_color = (age < extreme_age) ? P::Color::Normal : P::Color::Warn,
            }};
        } else {
            return {};
        }
    }
};

}// namespace djc::ui::pages
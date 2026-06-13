// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/StaticString.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/service/PeerScanningService.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerDetailPage.hpp"

namespace djc::ui::pages {

struct PeerExplorerPage : UI::Page {

    explicit PeerExplorerPage(
        UI &ui,
        UI::Page &root,
        transport::TransportLink &transport_link,
        service::PeerScanningService &peer_scanner,
        PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{ui, "Peer Explorer"},
        _ui{ui},
        _transport_link{transport_link},
        _peer_scanner{peer_scanner},
        _peer_favorites_registry{peer_favorites_registry},
        _peer_detail_page{ui, *this, _transport_link, _peer_favorites_registry},
        _layout{{
            &root.link(),
            &_primary_connection_status_button,
            &_available_label,
        }} {
        for (auto i = 0u; i < _peer_displays.size(); i += 1) {
            _peer_displays[i].callback([this](const transport::PeerAddress &address) -> void {
                _peer_detail_page.bindPeer(address);
                _ui.bindPage(_peer_detail_page);
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

        if (_transport_link.activePeerAddress().isSome()) {
            (void) _connection_button_buffer.format("%s", _transport_link.activePeerAddress().unwrap().toString().data());
            _primary_connection_status_button.label(_connection_button_buffer.view());
            _primary_connection_status_button.foreground(UI::Color::Normal);
            _primary_connection_status_button.background(UI::Color::Success);
        } else {
            _primary_connection_status_button.label("Disconnected");
            _primary_connection_status_button.foreground(UI::Color::Disabled);
            _primary_connection_status_button.background(UI::Color::Normal);
        }

        const auto available_peers = _peer_scanner.peers();
        (void) _available_label_buffer.format(" Available: %d", available_peers.size());
        _available_label.value(_available_label_buffer.view());

        for (auto i = 0u; i < available_peers.size(); i += 1) {
            const auto &entry = available_peers[i];
            _peer_displays[i].state(createPeerDisplayState(entry, now));

            if (entry.isSome()) {
                constexpr auto extreme_age_factor{0.75f};
                const auto extreme_age = _peer_scanner.config().entry_max_life_time * extreme_age_factor;
                const auto age = now - entry.unwrap().last_seen;

                _peer_displays[i].foreground((age < extreme_age) ? UI::Color::Primary : UI::Color::Warning);
            }
        }

        widgets(layout(available_peers.size()));
        update();
    }

private:
    static constexpr auto peer_display_start_index{3u};

    UI &_ui;
    transport::TransportLink &_transport_link;
    service::PeerScanningService &_peer_scanner;
    PeerFavoritesRegistry &_peer_favorites_registry;
    kf::math::Timer::Config _redraw_timer_config{
        .period = 500,
    };
    kf::math::Timer _redraw_timer{_redraw_timer_config};

    kf::memory::StaticString<64> _available_label_buffer{}, _connection_button_buffer{};

    UI::Button _primary_connection_status_button{{}};
    UI::Display<kf::memory::StringView> _available_label{_available_label_buffer.view()};
    kf::memory::Array<UI::PeerDisplay, service::PeerScanningService::max_entries> _peer_displays{};

    kf::memory::Array<UI::Widget *, (peer_display_start_index + service::PeerScanningService::max_entries)> _layout;

    // child pages
    PeerDetailPage _peer_detail_page;

    kf::Slice<UI::Widget *> layout(kf::usize displayed_peers) noexcept {
        return kf::Slice<UI::Widget *>{_layout.data(), _layout.size()}.first(peer_display_start_index + displayed_peers);
    }

    kf::Option<UI::PeerDisplay::State> createPeerDisplayState(const kf::TrivialOption<service::PeerScanningService::Entry> &entry, kf::math::Milliseconds now) const noexcept {
        using P = UI::PeerDisplay;

        const auto map_record = [](kf::Option<const PeerFavoritesRegistry::Entry &> record) -> kf::Option<kf::memory::StringView> {
            if (record.isSome()) {
                const auto &name = record.unwrap().name;
                return kf::some(kf::memory::StringView{name.data(), name.size()});
            } else {
                return kf::none;
            }
        };

        if (entry.isSome()) {
            return kf::some(P::State{
                .address = entry.unwrap().address,
                .name = map_record(_peer_favorites_registry.get(entry.unwrap().address)),
            });
        } else {
            return kf::none;
        }
    }
};

}// namespace djc::ui::pages
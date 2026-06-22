// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/StaticString.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerFavoritePage.hpp"

namespace djc::ui::pages {

struct PeerDetailPage final : UI::Page {

    explicit PeerDetailPage(
        UI &ui,
        UI::Page &root,
        transport::TransportLink &transport_link,
        PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{ui},
        _root{root},
        _transport_link{transport_link},
        _peer_favorites_registry{peer_favorites_registry},
        _layout{{
            &root.link(),
            &_connection_button,
            &_peer_favorite_button,
        }},
        _peer_favorite_page{ui, *this, peer_favorites_registry}

    {
        widgets(_layout.slice());

        _connection_button.callback([this]() -> void {
            if (_peer_address.isNone()) { return; }

            if (_transport_link.connect(_peer_address.unwrap())) {
                _ui.activePage(_root);
            } else {
                _connection_button.label("Failed to connect");
                _connection_button.style(UI::Style{
                    .background_color = UI::Color::Error,
                });
            }
        });

        _peer_favorite_button.hint("Open peer favorites edit");
        _peer_favorite_button.callback([this]() -> void {
            if (_peer_address.isNone()) { return; }
            this->label("Back");
            _peer_favorite_page.bindPeer(_peer_address.unwrap());
            _ui.activePage(_peer_favorite_page);
            _ui.requestRender();
        });
    }

    void bindPeer(const transport::PeerAddress &address) noexcept {
        _peer_address = kf::someTrivial(address);
    }

    void onEntry() noexcept override {
        _connection_button.label("Connect");
        _connection_button.style(UI::Style{
            .foreground_color = UI::Color::Primary,
        });

        if (_peer_address.isSome()) {
            _label_buffer = _peer_address.unwrap().toString();
            this->label(_label_buffer.view());

            _peer_favorite_button.label(_peer_favorites_registry.exists(_peer_address.unwrap()) ? "Edit" : "Add to favorites");
        }
    }

private:
    // state

    UI::Page &_root;
    transport::TransportLink &_transport_link;
    kf::TrivialOption<transport::PeerAddress> _peer_address{};
    PeerFavoritesRegistry &_peer_favorites_registry;

    // widgets

    transport::PeerAddress::StringType _label_buffer{};
    UI::Button _connection_button{{}}, _peer_favorite_button{{}};

    kf::memory::Array<UI::Widget *, 3> _layout;

    // child pages

    PeerFavoritePage _peer_favorite_page;
};

}// namespace djc::ui::pages

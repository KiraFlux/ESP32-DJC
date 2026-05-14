// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"
#include "djc/ui/pages/PeerFavoritePage.hpp"

namespace djc::ui::pages {

struct PeerDetailPage final : UI::Page {

    explicit PeerDetailPage(
        UI::Page &root,
        transport::TransportLink &transport_link,
        PeerFavoritesRegistry &peer_favorites_registry) noexcept :
        Page{{}},
        _peer_favorites_registry{peer_favorites_registry},
        _layout{{
            &root.link(),
            &_connection_button,
            &_peer_favorite_button,
        }}

    {
        widgets({_layout.data(), _layout.size()});

        _connection_button.callback([this, &transport_link, &root]() -> void {
            if (not _peer_address.hasValue()) { return; }

            if (transport_link.connect(_peer_address.value())) {
                UI::instance().bindPage(root);
            } else {
                _connection_button.label("Failed to connect");
            }
        });

        _peer_favorite_button.callback([this]() -> void {
            if (not _peer_address.hasValue()) { return; }
            this->label("Back");
            _peer_favorite_page.bindPeer(_peer_address.value());
            UI::instance().bindPage(_peer_favorite_page);
            UI::instance().addEvent(UI::Event::update());
        });
    }

    void bindPeer(const transport::PeerAddress &address) noexcept {
        _peer_address.value(address);
    }

    void onEntry() noexcept override {
        _connection_button.label("Connect");
        if (_peer_address.hasValue()) {
            _label_buffer = _peer_address.value().toString();
            this->label(_label_buffer.view());
    
            _peer_favorite_button.label(_peer_favorites_registry.exists(_peer_address.value()) ? "Edit" : "Add to favorites");
        }
    }

private:
    // state

    kf::Option<transport::PeerAddress> _peer_address{};
    PeerFavoritesRegistry &_peer_favorites_registry;

    // widgets

    transport::PeerAddress::ReprString _label_buffer{};
    UI::Button _connection_button{{}}, _peer_favorite_button{{}};

    kf::memory::Array<UI::Widget *, 3> _layout;

    // child pages

    PeerFavoritePage _peer_favorite_page{*this, _peer_favorites_registry};
};

}// namespace djc::ui::pages

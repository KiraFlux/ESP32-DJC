// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>

#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct PeerDetailPage final : UI::Page {

    explicit PeerDetailPage(UI::Page &root, transport::TransportLink &transport_link) noexcept :
        Page{"Oh no!"},
        _layout{{
            &root.link(),
            &_connection_button,
        }}

    {
        widgets({_layout.data(), _layout.size()});

        _connection_button.callback([this, &transport_link]() -> void {
            if (_peer_address.hasValue()) {
                (void) transport_link.connect(_peer_address.value());
            }
        });
    }

    void peerAddress(const transport::PeerAddress &new_peer_address) noexcept {
        _peer_address.value(new_peer_address);
        _label_buffer = new_peer_address.toString();
        this->label(_label_buffer.view());
    }

private:
    kf::Option<transport::PeerAddress> _peer_address{};

    transport::PeerAddress::ReprString _label_buffer{};
    UI::Button _connection_button{"Connect"};
    kf::memory::Array<UI::Widget *, 2> _layout;
};

}// namespace djc::ui::pages

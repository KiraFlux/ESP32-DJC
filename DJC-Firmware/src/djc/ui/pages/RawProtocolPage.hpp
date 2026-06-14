// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/Slice.hpp>
#include <kf/memory/StaticString.hpp>

#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/transport/TransportLink.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

struct RawProtocolPage : UI::Page {
    explicit RawProtocolPage(
        UI &ui,
        UI::Page &root,
        protocol::ProtocolRegistry &protocol_registry,
        protocol::ProtocolLink &protocol_link,
        transport::TransportLink &transport_link) noexcept :
        Page{ui, "Raw Protocol"}, _protocol_registry{protocol_registry}, _protocol_link{protocol_link}, _transport_link{transport_link},
        _message_input{ui.createTextInput({_message.data(), _message.size()})},
        _layout{{
            &root.link(),
            &_message_input,
            &_send_button,
        }} {
        widgets({_layout.data(), _layout.size()});

        _message_input.hint("Edit message");

        _send_button.callback([this]() {
            kf::memory::StringView s{_message.data(), _message.size()};
            s = s.sub(0, s.find('\0').unwrapOr(s.size()));

            logger.debug(s);

            (void) _transport_link.send({reinterpret_cast<const kf::u8 *>(s.data()), s.size()});
        });
        _send_button.hint("Send raw buffer as is");
    }

    void onEntry() noexcept override {
        _protocol_link.protocol(_protocol_registry.raw());

        _protocol_registry.raw().callback([](kf::Slice<const kf::u8> buffer) {
            logger.info(
                kf::memory::StaticString<64>::formatted(
                    "Got %d bytes from primary peer",
                    buffer.size())
                    .view());
        });
    }

    void onExit() noexcept override {
        _protocol_registry.raw().callback(kf::none);
    }

private:
    static constexpr auto logger{kf::Logger::create("RawControlPage")};

    kf::memory::Array<char, 200> _message{};
    protocol::ProtocolRegistry &_protocol_registry;
    protocol::ProtocolLink &_protocol_link;
    transport::TransportLink &_transport_link;

    // widgets

    UI::TextInput _message_input;
    UI::Button _send_button{"Send"};

    kf::memory::Array<UI::Widget *, 3> _layout;
};

}// namespace djc::ui::pages
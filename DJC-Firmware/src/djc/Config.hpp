// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/PeerScanner.hpp"
#include "djc/Periphery.hpp"
#include "djc/input/InputHandler.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc {

struct Config {
    static constexpr auto latest_version{7u}, max_peer_favorites{8u};

    kf::u16 version;

    protocol::ProtocolRegistry::Mode init_protocol_mode;
    kf::memory::Array<char, 16> device_name;
    kf::memory::Array<kf::Option<PeerFavoritesRegistry::Entry>, max_peer_favorites> peer_favorites;

    Periphery::Config periphery;
    InputHandler::Config input_handler;

    transport::TransportLink::Config transport_link;
    PeerScanner::Config peer_scanner;

    protocol::ProtocolLink::Config protocol_link;
    protocol::ProtocolRegistry::Config protocol_registry;

    [[nodiscard]] constexpr kf::memory::StringView deviceName() const noexcept {
        return kf::memory::StringView{device_name.data(), device_name.size()};
    }

    [[nodiscard]] bool isLatestVersion() const noexcept { return version == latest_version; }

    static constexpr Config defaults() noexcept {
        return Config{
            .version = latest_version,

            .init_protocol_mode = protocol::ProtocolRegistry::Mode::Mavlink,
            .device_name = {"ESP32-DJC"},
            .peer_favorites = {},

            .periphery = Periphery::Config::defaults(),
            .input_handler = InputHandler::Config::defaults(),

            .transport_link = transport::TransportLink::Config::defaults(),
            .peer_scanner = PeerScanner::Config::defaults(),

            .protocol_link = protocol::ProtocolLink::Config::defaults(),
            .protocol_registry = protocol::ProtocolRegistry::Config::defaults(),
        };
    }
};

}// namespace djc
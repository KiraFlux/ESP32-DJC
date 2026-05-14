// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// lib
#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Array.hpp>

// periphery
#include "djc/Periphery.hpp"

// transport
#include "djc/transport/Kind.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"

// protocol
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"

// services
#include "djc/AutoConnectService.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/PeerScanner.hpp"
#include "djc/input/InputHandler.hpp"

namespace djc {

/// @brief Persistent configuration structure stored in NVS.
struct Config {
    static constexpr auto
        latest_version{9u},
        max_peer_favorites{8u};

    kf::u16 version;

    // user
    protocol::ProtocolRegistry::Mode init_protocol_mode;
    transport::Kind init_transport_kind;
    kf::memory::Array<char, 16> device_name;
    kf::memory::Array<kf::Option<PeerFavoritesRegistry::Entry>, max_peer_favorites> peer_favorites;

    // periphery
    Periphery::Config periphery;

    // transport
    transport::TransportLink::Config transport_link;

    // protocol
    protocol::ProtocolLink::Config protocol_link;
    protocol::ProtocolRegistry::Config protocol_registry;

    // services
    InputHandler::Config input_handler;
    PeerScanner::Config peer_scanner;
    AutoConnectService::Config auto_connect_service;

    [[nodiscard]] bool isLatestVersion() const noexcept { return version == latest_version; }

    static constexpr Config defaults() noexcept {
        return Config{
            .version = latest_version,

            .init_protocol_mode = protocol::ProtocolRegistry::Mode::Mavlink,
            .init_transport_kind = transport::Kind::EspNow,
            .device_name = {"ESP32-DJC"},
            .peer_favorites = {},

            .periphery = Periphery::Config::defaults(),

            .transport_link = transport::TransportLink::Config::defaults(),

            .protocol_link = protocol::ProtocolLink::Config::defaults(),
            .protocol_registry = protocol::ProtocolRegistry::Config::defaults(),

            .input_handler = InputHandler::Config::defaults(),
            .peer_scanner = PeerScanner::Config::defaults(),
            .auto_connect_service = AutoConnectService::Config::defaults(),
        };
    }
};

}// namespace djc
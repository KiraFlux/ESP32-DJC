// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/Periphery.hpp"
#include "djc/config/Config.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/service/AutoConnectService.hpp"
#include "djc/service/InputHandler.hpp"
#include "djc/service/PeerScanningService.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::config {

/// @brief Device-related configutation
struct DeviceConfig : Config<DeviceConfig, 0> {

    // periphery

    Periphery::Config periphery;

    // network

    transport::TransportLink::Config transport_link;
    protocol::ProtocolLink::Config protocol_link;
    protocol::ProtocolRegistry::Config protocol_registry;

    // service configs

    service::InputHandler::Config input_handler;
    service::PeerScanningService::Config peer_scanner;
    service::AutoConnectService::Config auto_connect_service;

private:
    KF_IMPL_RESETTABLE(DeviceConfig);
    void resetImpl() noexcept {
        periphery.reset();

        transport_link.reset();
        protocol_link.reset();
        protocol_registry.reset();

        input_handler = service::InputHandler::Config::defaults();
        peer_scanner.reset();
        auto_connect_service = service::AutoConnectService::Config::defaults();
    }
};

}// namespace djc::config
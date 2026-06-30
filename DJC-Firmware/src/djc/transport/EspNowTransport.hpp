// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/network/EspNow.hpp>
#include <kf/network/MacAddress.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

/// @brief ESP‑NOW transport implementation
/// @note Manages ESP‑NOW peer connections. uses dedicated active peer for communication
struct EspNowTransport : Transport, kf::mixin::Initable<EspNowTransport, bool()> {

    [[nodiscard]] bool send(kf::Slice<const kf::u8> buffer) noexcept override {
        if (_active_peer.isSome()) {
            return _active_peer.unwrap().writeBuffer(buffer).isOk();
        } else {
            return false;
        }
    }

protected:
    /// @brief Establish a connection to a peer
    /// @param address The peer's address (must be of kind `EspNow`)
    /// @return true on success, false on failure
    /// @note Adds the peer to ESP‑NOW and sets up a receive callback
    [[nodiscard]] bool doConnect(const PeerAddress &address) noexcept override {
        if (address.kind() != Kind::EspNow) { return false; }

        auto peer_result = EspNow::Peer::create(EspNow::Peer::Config{
            .mac_address = address.mac(),
            .wifi_interface_sta = true,
        });

        if (peer_result.isError()) {
            logger.error(LogString::formatted("Connect to '%s' failed: %s", address.mac().toString().data(), peer_result.error().toString().data()).view());
            return false;
        }

        _active_peer = kf::some(std::move(peer_result.ok()));

        logger.info(LogString::formatted("Connected: primary peer set '%s'", address.mac().toString().data()).view());
        return true;
    }

    /// @brief Disconnect from the current peer
    /// @note Removes the peer from ESP‑NOW and clears internal state
    void doDisconnect() noexcept override {
        if (not connected()) {
            logger.warn("Disconnect failed: No active peer");
            return;
        }

        auto &peer = _active_peer.unwrap();
        if (not peer.exist()) {
            logger.error("Disconnect failed: Peer not exit");
            return;
        }

        _active_peer.reset();
        logger.info("Disconnected: OK");
    }

private:
    using EspNow = kf::network::EspNow;
    using LogString = kf::memory::StaticString<128>;

    static constexpr auto logger{kf::Logger::create("EspNowTransport")};

    kf::Option<EspNow::Peer> _active_peer{kf::none};

    KF_IMPL_INITABLE(EspNowTransport, bool());
    bool initImpl() noexcept {
        logger.info("init");

        auto &espnow = EspNow::instance();

        const auto result = espnow.init();
        if (result.isError()) {
            logger.error(LogString::formatted("Failed to initialize ESP-NOW: %s", result.error().toString().data()));
            return false;
        }

        espnow.callback([this](const kf::network::MacAddress &mac, kf::Slice<const kf::u8> buffer) {
            if (_active_peer.isSome() and _active_peer.unwrap().mac() == mac) {
                invokeReceive(buffer);
            } else {
                invokeReceiveForeign(PeerAddress::fromEspnowMac(mac), buffer);
            }
        });

        return true;
    }
};

}// namespace djc::transport
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/network/MacAddress.hpp>

#include "kf-patch/EspNow.hpp"// patched version

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

/// @brief ESP‑NOW transport implementation.
/// @note Manages ESP‑NOW peer connections. uses dedicated active peer for communication.
struct EspNowTransport : Transport, kf::mixin::Initable<EspNowTransport, bool> {

    [[nodiscard]] bool send(kf::Slice<const kf::u8> buffer) noexcept override {
        if (_active_peer.isSome()) {
            return _active_peer.unwrap().writeBuffer(buffer).isOk();
        } else {
            return false;
        }
    }

private:
    using EspNow = kf::network::EspNow;
    using LogString = kf::memory::StaticString<128>;

    static constexpr auto logger{kf::Logger::create("EspNowTransport")};

    /// @brief MAC address used for ESP‑NOW broadcast.
    static constexpr kf::network::MacAddress broadcast_mac_address{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

protected:
    /// @brief Establish a connection to a peer.
    /// @param address The peer's address (must be of kind `EspNow`).
    /// @return true on success, false on failure.
    /// @note Adds the peer to ESP‑NOW and sets up a receive callback.
    [[nodiscard]] bool doConnect(const PeerAddress &address) noexcept override {
        if (address.kind() != Kind::EspNow) { return false; }

        _active_peer = addPeer(address.mac());
        if (_active_peer.isNone()) { return false; }

        logger.info("Connected: OK");
        return true;
    }

    /// @brief Disconnect from the current peer.
    /// @note Removes the peer from ESP‑NOW and clears internal state.
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
    kf::Option<EspNow::Peer> _active_peer{};

    static auto addPeer(const kf::network::MacAddress &mac) noexcept -> kf::Option<EspNow::Peer> {
        auto peer_result = EspNow::Peer::create(EspNow::Peer::Config{
            .mac_address = mac,
            .wifi_interface_sta = true,
        });

        if (peer_result.isOk()) {
            logger.info(LogString::formatted("Peer '%s' added", mac.toString().data()).view());
            return kf::some(std::move(peer_result.ok()));
        } else {
            logger.error(LogString::formatted("Failed to add peer [%s] :%s", mac.toString().data(), peer_result.error().toString().data()).view());
            return kf::none;
        }
    }

    static void delPeer(EspNow::Peer &peer) noexcept {
        const auto result = peer.del();
        if (result.isError()) {
            logger.error(LogString::formatted("Failed to delete peer [%s] : %s", peer.mac().toString().data(), result.error().toString().data()).view());
        }
    }

    KF_IMPL_INITABLE(EspNowTransport, bool);
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

        logger.debug("init: ok");
        return true;
    }
};

}// namespace djc::transport
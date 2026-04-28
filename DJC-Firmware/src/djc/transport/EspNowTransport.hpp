// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/network/EspNow.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/Transport.hpp"

namespace djc::transport {

/// @brief ESP‑NOW transport implementation.
/// @note Manages ESP‑NOW peer connections. Uses a broadcast peer for discovery and a dedicated active peer for communication.
struct EspNowTransport : Transport, kf::mixin::Initable<EspNowTransport, bool> {

    [[nodiscard]] bool send(kf::memory::Slice<const kf::u8> buffer) noexcept override {
        if (_active_peer.hasValue()) {
            return _active_peer.value().writeBuffer(buffer).isOk();
        } else {
            return false;
        }
    }

private:
    using EspNow = kf::network::EspNow;
    using LogString = kf::memory::ArrayString<128>;

    static constexpr auto logger{kf::Logger::create("EspNowTransport")};

    /// @brief MAC address used for ESP‑NOW broadcast.
    static constexpr EspNow::Mac broadcast_mac_address{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

protected:
    /// @brief Establish a connection to a peer.
    /// @param address The peer's address (must be of kind `EspNow`).
    /// @return true on success, false on failure.
    /// @note Adds the peer to ESP‑NOW and sets up a receive callback.
    [[nodiscard]] bool doConnect(const PeerAddress &address) noexcept override {
        if (address.kind() != Kind::EspNow) { return false; }

        _active_peer = addPeer(address.mac());
        if (not _active_peer.hasValue()) { return false; }

        const auto receive_setup_result = _active_peer.value().onReceive([this](kf::memory::Slice<const kf::u8> buffer) {
            invokeReceive(buffer);
        });

        if (receive_setup_result.isError()) {
            logger.error(
                LogString::formatted(
                    "Receive callback attachment failed: %s",
                    EspNow::stringFromError(receive_setup_result.error()))
                    .view());
            return false;
        }

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

        auto &peer = _active_peer.value();
        if (not peer.exist()) {
            logger.error("Disconnect failed: Peer not exit");
            return;
        }

        delPeer(peer);

        _active_peer = {};
        logger.info("Disconnected: OK");
    }

private:
    kf::Option<EspNow::Peer>
        _broadcast_peer{},///< Broadcast peer for discovery.
        _active_peer{};   ///< Currently connected peer

    static kf::Option<EspNow::Peer> addPeer(const EspNow::Mac &mac) noexcept {
        auto peer_result = EspNow::Peer::add(mac);
        if (peer_result.isError()) {
            logger.error(
                LogString::formatted(
                    "Failed to add peer [%s] :%s",
                    EspNow::stringFromMac(mac).data(),
                    EspNow::stringFromError(peer_result.error()))
                    .view());
            return {};
        }

        logger.info(LogString::formatted("Peer '%s' added", EspNow::stringFromMac(mac).data()).view());
        return {std::move(peer_result.value())};
    }

    static void delPeer(EspNow::Peer &peer) noexcept {
        const auto result = peer.del();
        if (result.isError()) {
            logger.error(
                LogString::formatted(
                    "Failed to delete peer [%s] : %s",
                    EspNow::stringFromMac(peer.mac()).data(),
                    EspNow::stringFromError(result.error()))
                    .view());
            return;
        }
    }

    // impl
    using This = EspNowTransport;

    KF_IMPL_INITABLE(This, bool);
    bool initImpl() noexcept {
        logger.info("init");

        auto &espnow = EspNow::instance();

        const auto result = espnow.init();
        if (result.isError()) {
            logger.error(LogString::formatted("Failed to initialize ESP-NOW: %s", EspNow::stringFromError(result.error())));
            return false;
        }

        espnow.onReceiveFromUnknown([this](const EspNow::Mac &mac, kf::memory::Slice<const kf::u8> buffer){
            invokeReceiveForeign(PeerAddress{mac}, buffer);    
        });

        _broadcast_peer = addPeer(broadcast_mac_address);

        logger.debug("init: ok");
        return true;
    }
};

}// namespace djc::transport
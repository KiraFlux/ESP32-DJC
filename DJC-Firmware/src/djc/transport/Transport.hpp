// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Function.hpp>
#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/transport/PeerAddress.hpp"

namespace djc::transport {

/// @brief Abstract transport layer for peer-to-peer communication.
///
/// Manages the lifecycle of a connection to a single peer. Subclasses implement
/// the actual communication hardware
///
/// @note The connection state (`_active_peer`) is managed by the base class.
struct Transport : kf::mixin::NonCopyable {

    /// @brief Callback invoked when data is received from a peer.
    using ReceiveCallback = kf::Function<void(const PeerAddress &, kf::memory::Slice<const kf::u8>)>;

    /// @brief Send raw data to the currently connected peer.
    /// @param buffer Raw payload.
    /// @return true if the data was sent successfully, false otherwise.
    /// @note Must only be called when connected.
    [[nodiscard]] virtual bool send(kf::memory::Slice<const kf::u8> buffer) const noexcept = 0;

    /// @brief Register a callback for incoming data.
    /// @param callback Functor invoked on each received packet.
    /// @note The callback is set before a connection is established.
    virtual void onReceive(ReceiveCallback &&callback) noexcept = 0;

protected:
    /// @brief Hardware‑specific connection procedure.
    /// @param addr Address of the peer.
    /// @return true on success, false on failure.
    /// @note Implementations should handle invalid or incompatible address types.
    virtual bool doConnect(const PeerAddress &address) noexcept = 0;

    /// @brief Hardware‑specific disconnection procedure.
    /// @note Called even if not connected; implementations must be safe.
    virtual void doDisconnect() noexcept = 0;

public:
    /// @brief Check whether the transport is currently connected to a peer.
    /// @return true if a peer is active, false otherwise.
    [[nodiscard]] bool connected() const noexcept { return _active_peer.hasValue(); }

    /// @brief Get the address of the currently connected peer.
    /// @return Option containing the peer address if connected, empty Option otherwise.
    [[nodiscard]] const kf::Option<PeerAddress> &activePeer() const noexcept { return _active_peer; }

    /// @brief Connect to a remote peer.
    /// @param peer_address Address of the peer to connect to.
    /// @return true on success, false on failure.
    /// @note If already connected, the method will return false. Disconnect explicitly first.
    [[nodiscard]] bool connect(const PeerAddress &address) noexcept {
        if (connected()) { return false; }
        if (not doConnect(address)) { return false; }

        _active_peer = address;

        return true;
    }

    /// @brief Disconnect from the current peer.
    /// @note Safe to call even if not connected.
    void disconnect() noexcept {
        doDisconnect();

        _active_peer = {};
    }

private:
    kf::Option<PeerAddress> _active_peer{};
};

}// namespace djc::transport
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Function.hpp>
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/primitives.hpp>

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
    using ReceiveCallback = kf::Function<void(const PeerAddress &, kf::Slice<const kf::u8>)>;

    /// @brief Send raw data to the currently connected peer.
    /// @param buffer Raw payload.
    /// @return true if the data was sent successfully, false otherwise.
    /// @note Must only be called when connected.
    [[nodiscard]] virtual bool send(kf::Slice<const kf::u8> buffer) noexcept = 0;

protected:
    /// @brief Hardware‑specific connection procedure.
    /// @param addr Address of the peer.
    /// @return true on success, false on failure.
    /// @note Implementations should handle invalid or incompatible address types.
    [[nodiscard]] virtual bool doConnect(const PeerAddress &address) noexcept = 0;

    /// @brief Hardware‑specific disconnection procedure.
    /// @note Called even if not connected; implementations must be safe.
    virtual void doDisconnect() noexcept = 0;

public:
    /// @brief Register a callback for incoming data.
    /// @param callback Functor invoked on each received packet.
    void onReceive(ReceiveCallback &&callback) noexcept {
        _receive_callback = kf::some(std::move(callback));
    }

    /// @brief Register a callback for incoming data from non-primary peer
    /// @param callback Functor invoked on each received packet.
    void onReceiveForeign(ReceiveCallback &&callback) noexcept {
        _broadcast_receive_callback = kf::some(std::move(callback));
    }

    /// @brief Check whether the transport is currently connected to a peer.
    /// @return true if a peer is active, false otherwise.
    [[nodiscard]] bool connected() const noexcept {
        return _active_peer_address.isSome();
    }

    /// @brief Get the address of the currently connected peer.
    /// @return Option containing the peer address if connected, empty Option otherwise.
    [[nodiscard]] kf::Option<const PeerAddress &> activePeerAddress() const noexcept {
        return _active_peer_address.isNone() ? kf::none : kf::someRef(_active_peer_address.unwrap());
    }

    /// @brief Connect to a remote peer.
    /// @param peer_address Address of the peer to connect to.
    /// @return true on success, false on failure.
    [[nodiscard]] bool connect(const PeerAddress &address) noexcept {
        if (connected()) {
            if (_active_peer_address.unwrap() == address) { return true; }// already on this peer

            disconnect();
        }

        if (not doConnect(address)) { return false; }

        _active_peer_address = kf::someTrivial(address);

        return true;
    }

    /// @brief Disconnect from the current peer.
    /// @note Safe to call even if not connected.
    void disconnect() noexcept {
        doDisconnect();
        _active_peer_address.reset();
    }

protected:
    void invokeReceive(kf::Slice<const kf::u8> buffer) noexcept {
        if (_active_peer_address.isSome() and _receive_callback.isSome()) {
            _receive_callback.unwrap()(_active_peer_address.unwrap(), buffer);
        }
    }

    void invokeReceiveForeign(const PeerAddress &address, kf::Slice<const kf::u8> buffer) noexcept {
        if (_broadcast_receive_callback.isSome()) {
            _broadcast_receive_callback.unwrap()(address, buffer);
        }
    }

private:
    kf::Option<ReceiveCallback> _receive_callback{kf::none}, _broadcast_receive_callback{kf::none};
    kf::TrivialOption<PeerAddress> _active_peer_address{kf::none};
};

}// namespace djc::transport
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/ArrayString.hpp>
#include <kf/network/EspNow.hpp>

#include "djc/transport/Kind.hpp"

namespace djc::transport {

/// @brief Unified address of a peer, independent of the underlying transport.
///
/// Holds either a MAC address (ESP‑NOW).
/// The active kind is stored in a tag field; the union contains the actual address.
struct PeerAddress {

    using ReprString = kf::memory::ArrayString<32>;

    /// @brief Construct an ESP‑NOW peer address from a MAC.
    /// @param mac 6‑byte MAC address (EspNow::Mac).
    explicit constexpr PeerAddress(const kf::network::EspNow::Mac &mac) noexcept :
        _kind{Kind::EspNow}, _mac{mac} {}

    /// @brief Return the kind of transport this address belongs to.
    [[nodiscard]] Kind kind() const noexcept { return _kind; }

    /// @brief Return the stored MAC address.
    /// @note available if kind() == Kind::EspNow.
    [[nodiscard]] kf::network::EspNow::Mac mac() const noexcept { return _mac; }

    /// @brief Equality comparison.
    /// @note Two addresses are equal if they have the same kind and the same underlying value.
    [[nodiscard]] bool operator==(const PeerAddress &other) const noexcept {
        if (other.kind() != _kind) { return false; }

        switch (_kind) {
            case Kind::EspNow:
                return other.mac() == this->mac();

            default:
                return false;
        }
    }

    /// @brief Inequality comparison (delegates to operator==).
    [[nodiscard]] bool operator!=(const PeerAddress &other) const noexcept { return not this->operator==(other); }

    /// @brief Get String Representation
    [[nodiscard]] ReprString toString() const noexcept {
        switch (_kind) {
            case Kind::EspNow:
                return ReprString::formatted("<EspNow: %s>", kf::network::EspNow::stringFromMac(_mac).data());
        }
        return {};
    }

private:
    Kind _kind;

    union {
        kf::network::EspNow::Mac _mac;
    };
};

}// namespace djc::transport
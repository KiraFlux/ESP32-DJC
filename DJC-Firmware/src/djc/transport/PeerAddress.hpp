// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/StaticString.hpp>
#include <kf/mixin/StringRepresentable.hpp>
#include <kf/network/EspNow.hpp>
#include <kf/network/MacAddress.hpp>

#include "djc/transport/Kind.hpp"

namespace djc::internal {

using PeerAddressStringType = kf::memory::StaticString<32>;

}

namespace djc::transport {

/// @brief Unified address of a peer, independent of the underlying transport.
///
/// Holds either a MAC address (ESP‑NOW).
/// The active kind is stored in a tag field; the union contains the actual address.
struct PeerAddress : kf::mixin::StringRepresentable<PeerAddress, internal::PeerAddressStringType> {

    /// @brief create an ESP‑NOW peer address from a MAC.
    /// @param mac 6‑byte MAC address (EspNow::Mac).
    static constexpr PeerAddress fromEspnowMac(const kf::network::MacAddress &mac) noexcept {
        PeerAddress ret{};
        ret._kind = Kind::EspNow,
        ret._mac = mac;
        return ret;
    }

    /// @brief Return the kind of transport this address belongs to.
    [[nodiscard]] Kind kind() const noexcept {
        return _kind;
    }

    /// @brief Return the stored MAC address.
    /// @note available if kind() == Kind::EspNow.
    [[nodiscard]] kf::network::MacAddress mac() const noexcept {
        return _mac;
    }

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
    [[nodiscard]] bool operator!=(const PeerAddress &other) const noexcept {
        return not this->operator==(other);
    }

private:
    Kind _kind;

    union {
        kf::network::MacAddress _mac;
    };

    using S = internal::PeerAddressStringType;
    
    KF_IMPL_STRING_REPRESENTABLE(PeerAddress, S);
    auto toStringImpl() const noexcept {
        switch (_kind) {
            case Kind::EspNow:
                return S::formatted("%s@EspNow", _mac.toString().data());

            default:
                return S{};
        }
    }
};

}// namespace djc::transport
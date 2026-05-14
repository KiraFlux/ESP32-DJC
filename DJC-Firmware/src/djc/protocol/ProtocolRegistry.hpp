// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/protocol/MavlinkProtocol.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/protocol/RawProtocol.hpp"

namespace djc::protocol {

namespace internal {

/// @brief Configuration container for the ProtocolRegistry.
struct ProtocolRegistryConfig final : kf::mixin::NonCopyable {

    MavlinkProtocol::Config mavlink;///< MAVLink protocol configuration

    [[nodiscard]] static constexpr ProtocolRegistryConfig defaults() noexcept {
        return ProtocolRegistryConfig{
            .mavlink = MavlinkProtocol::Config::defaults(),
        };
    }
};

}// namespace internal

/// @brief Storage for all available protocol implementations.
struct ProtocolRegistry final :

    kf::mixin::NonCopyable,
    kf::mixin::Configurable<internal::ProtocolRegistryConfig>

{
    using Config = internal::ProtocolRegistryConfig;

    /// @brief Available protocol modes.
    enum class Mode : char {
        Raw = 0x00,    ///< Raw binary protocol (sends ManualInput as-is)
        Mavlink = 0x01,///< MAVLink protocol (sends MANUAL_CONTROL and HEARTBEAT)
    };

    using kf::mixin::Configurable<Config>::Configurable;

    /// @brief Retrieve a protocol instance by mode.
    /// @param mode Requested protocol mode
    /// @return Reference to the corresponding protocol object.
    [[nodiscard]] Protocol &get(Mode mode) noexcept {
        switch (mode) {
            case Mode::Mavlink:
                return _mavlink_protocol;

            case Mode::Raw:
            default:
                return _raw_protocol;
        }
    }

    /// @brief Direct access to the Raw protocol instance.
    [[nodiscard]] RawProtocol &raw() noexcept { return _raw_protocol; }

    /// @brief Direct access to the MAVLink protocol instance.
    [[nodiscard]] MavlinkProtocol &mavlink() noexcept { return _mavlink_protocol; }

private:
    RawProtocol _raw_protocol{};
    MavlinkProtocol _mavlink_protocol{this->config().mavlink};
};

}// namespace djc::protocol
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/aliases.hpp>
#include <kf/memory/StringView.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/protocol/MavlinkProtocol.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/protocol/RawProtocol.hpp"

namespace djc::protocol {

namespace internal {

struct ProtocolRegistryConfig final : kf::mixin::NonCopyable {

    MavlinkProtocol::Config mavlink;

    [[nodiscard]] static constexpr ProtocolRegistryConfig defaults() noexcept {
        return ProtocolRegistryConfig{
            .mavlink = MavlinkProtocol::Config::defaults(),
        };
    }
};

}// namespace internal

struct ProtocolRegistry final :

    kf::mixin::NonCopyable,
    kf::mixin::Configurable<internal::ProtocolRegistryConfig>

{
    using Config = internal::ProtocolRegistryConfig;

    enum class Mode : kf::u8 {
        Raw = 0x00,
        Mavlink = 0x01,
    };

    using kf::mixin::Configurable<Config>::Configurable;

    [[nodiscard]] Protocol &get(Mode mode) noexcept {
        switch (mode) {
            case Mode::Mavlink:
                return _mavlink_protocol;

            case Mode::Raw:
            default:
                return _raw_protocol;
        }
    }

    [[nodiscard]] RawProtocol &raw() noexcept { return _raw_protocol; }

    [[nodiscard]] MavlinkProtocol &mavlink() noexcept { return _mavlink_protocol; }

    [[nodiscard]] static constexpr kf::memory::StringView stringFromMode(Mode mode) noexcept { return (mode == Mode::Raw) ? "Raw" : "MavLink"; }

private:
    RawProtocol _raw_protocol{};
    MavlinkProtocol _mavlink_protocol{this->config().mavlink};
};

}// namespace djc::protocol
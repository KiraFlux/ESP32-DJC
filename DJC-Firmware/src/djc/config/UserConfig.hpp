// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/memory/Array.hpp>

#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/config/Config.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/transport/Kind.hpp"
#include "djc/ui/UI.hpp"

namespace djc::config {

/// @brief User Configuration
struct UserConfig : Config<UserConfig, 0> {

    static constexpr auto max_peer_favorites{8u};

    /// @brief Protocol mode that sent after init
    protocol::ProtocolRegistry::Mode init_protocol_mode;

    /// @brief Transport kind that sent after init
    transport::Kind init_transport_kind;

    /// @brief Human-readable string
    kf::memory::Array<char, 16> device_name;

    /// @brief Peer favorites registry entries
    kf::memory::Array<kf::TrivialOption<PeerFavoritesRegistry::Entry>, max_peer_favorites> peer_favorites;

    /// @brief UI Renderer mics config
    ui::UI::Traits::RendererImpl::Config ui_renderer;

private:
    KF_IMPL_RESETTABLE(UserConfig);
    void resetImpl() noexcept {
        init_protocol_mode = protocol::ProtocolRegistry::Mode::Mavlink;
        init_transport_kind = transport::Kind::EspNow;

        device_name = decltype(device_name){"ESP32-DJC"};

        peer_favorites = {};

        ui_renderer = ui::UI::Traits::RendererImpl::Config::defaults();
    }
};

}// namespace djc::config
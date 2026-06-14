// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <WiFi.h>

#include <kf/Logger.hpp>
#include <kf/mixin/Configurable.hpp>

#include "djc/Config.hpp"
#include "djc/PeerFavoritesRegistry.hpp"
#include "djc/service/AutoConnectService.hpp"
#include "djc/service/PeerScanningService.hpp"
#include "djc/system/System.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::system {

/// @brief System managing peer favorites, scanning and auto-connection.
/// @note Owns PeerFavoritesRegistry, PeerScanningService, AutoConnectService.
/// @note Depends on Config (readolny) and TransportLink (for scanning and connection).
/// @note On each poll, scans visible peers and triggers auto-connection to the most trusted visible favorite.
/// @note Peer favorites registry entries source should set extenally
struct PeerSystem : System<PeerSystem>, kf::mixin::Configurable<Config> {

    explicit PeerSystem(const Config &config, transport::TransportLink &transport_link) noexcept :
        kf::mixin::Configurable<Config>{config}, _transport_link{transport_link} {}

    /// @brief Get mutable access to peer favorites registry component
    PeerFavoritesRegistry &favoritesRegistry() noexcept {
        return _peer_favorites_registry;
    }

    /// @brief Get readonly access to peer favorites registry component
    constexpr const PeerFavoritesRegistry &favoritesRegistry() const noexcept {
        return _peer_favorites_registry;
    }

    /// @brief Get mutable access to peer scanning service component
    service::PeerScanningService &scanningService() noexcept {
        return _peer_scanning_service;
    }

    /// @brief Get readonly access to peer scanning service component
    constexpr const service::PeerScanningService &scanningService() const noexcept {
        return _peer_scanning_service;
    }

    /// @brief Get mutable access to auto‑connect service component
    service::AutoConnectService &autoConnectService() noexcept {
        return _auto_connect_service;
    }

    /// @brief Get readonly access to auto‑connect service component
    constexpr const service::AutoConnectService &autoConnectService() const noexcept {
        return _auto_connect_service;
    }

private:
    static constexpr auto logger{kf::Logger::create("PeerSystem")};

    transport::TransportLink &_transport_link;
    PeerFavoritesRegistry _peer_favorites_registry{};
    service::PeerScanningService _peer_scanning_service{this->config().peer_scanner, _transport_link};
    service::AutoConnectService _auto_connect_service{this->config().auto_connect_service, _transport_link};

    KF_IMPL_INITABLE(PeerSystem, bool);
    bool initImpl() noexcept {
        _peer_favorites_registry.init();
        _peer_scanning_service.init();

        _auto_connect_service.callback([this](const auto &address) -> void {
            logger.info("Auto Connect");
            (void) _transport_link.connect(address);
        });

        return true;
    }

    KF_IMPL_TIMED_POLLABLE(PeerSystem);
    void poll(kf::math::Milliseconds now) noexcept {
        _peer_scanning_service.poll(now);

        if (_auto_connect_service.config().enabled and _auto_connect_service.target().isNone()) {
            const auto favorites = _peer_favorites_registry.all();

            if (favorites.size() > 0) {
                auto most_trusted_favorite_index = 0u;

                for (auto index = 1u; index < favorites.size(); index += 1) {
                    if (favorites[index].isSome() and favorites[most_trusted_favorite_index].isSome() and favorites[index].unwrap().trust > favorites[most_trusted_favorite_index].unwrap().trust) {
                        most_trusted_favorite_index = index;
                    }
                }

                if (const auto &most_trusted = favorites[most_trusted_favorite_index]; most_trusted.isSome()) {
                    for (const auto &peer: _peer_scanning_service.peers()) {
                        if (peer.isSome() and peer.unwrap().address == most_trusted.unwrap().address) {
                            _auto_connect_service.target(most_trusted.unwrap().address);
                            break;
                        }
                    }
                }
            }
        }

        _auto_connect_service.poll(now);
    }
};

}// namespace djc::system
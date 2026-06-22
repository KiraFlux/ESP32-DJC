// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/primitives.hpp>

#include "djc/Config.hpp"
#include "djc/math.hpp"
#include "djc/service/Service.hpp"

namespace djc::service {

/// @brief Config service with delayed NVS operations
/// @note Requests are batched and executed on a 5-second timer from the main loop.
struct ConfigService : Service<ConfigService> {

    /// @brief Get readonly reference to the current configuration
    [[nodiscard]] constexpr const Config &config() const noexcept {
        return _storage.config;
    }

    /// @brief Get mutable reference to the current configuration
    [[nodiscard]] Config &config() noexcept {
        return _storage.config;
    }

    /// @brief Requests an deferred load of the config from NVS
    void requestLoad() noexcept {
        _load_requested = true;
        logger.debug("Load requested");
    }

    /// @brief Requests an deferred reset of the config to defaults
    void requestReset() noexcept {
        _reset_requested = true;
        logger.debug("Reset requested");
    }

    /// @brief Force sync now
    void sync() noexcept {
        using LogString = kf::memory::StaticString<64>;

        if (_load_requested) {
            _load_requested = false;

            logger.info("Loading config from NVS...");

            if (_storage.load()) {
                _stored_crc = crc();
                logger.info(LogString::formatted("Config loaded from NVS (CRC: %u)", _stored_crc).view());

                if (not _storage.config.isLatestVersion()) {
                    logger.warn("Config version is outdated");
                    requestReset();
                }
            } else {
                logger.error("Config load failed");
                requestReset();
            }
        }

        if (_reset_requested) {
            _reset_requested = false;
            logger.info("Config reset to defaults");

            _storage.config = djc::Config::defaults();
        }

        if (const auto current_crc = crc(); current_crc != _stored_crc) {
            logger.info(LogString::formatted("Config changed, saving (CRC: %u -> %u)...", _stored_crc, current_crc).view());

            if (_storage.save()) {
                _stored_crc = current_crc;
                logger.info("Config saved, CRC updated");
            } else {
                logger.error("Config save failed");
            }
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigService")};

    static constexpr kf::math::Timer::Config sync_timer_config{
        .period = 10'000,
    };

    kf::memory::Storage<Config> _storage{
        .key = "DC",
        .config = djc::Config::defaults(),
    };

    kf::math::Timer _sync_timer{sync_timer_config};

    kf::u32 _stored_crc{};

    bool _load_requested{false}, _reset_requested{false};

    [[nodiscard]] kf::u32 crc() const noexcept {
        return djc::math::crc32({reinterpret_cast<const kf::u8 *>(&_storage.config), sizeof(Config)});
    }

    KF_IMPL_TIMED_POLLABLE(ConfigService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_sync_timer.expired(now)) {
            _sync_timer.start(now);
            sync();
        }
    }
};

}// namespace djc::service
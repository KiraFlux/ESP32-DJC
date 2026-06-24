// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/primitives.hpp>

#include "djc/Config.hpp"
#include "djc/math.hpp"
#include "djc/memory/NVS.hpp"
#include "djc/service/Service.hpp"

namespace djc::service {

/// @brief Config service with delayed NVS operations
/// @note Requests are batched and executed on a 5-second timer from the main loop.
struct ConfigService : Service<ConfigService> {

    explicit constexpr ConfigService(const kf::math::Timer::Config &sync_timer_config) noexcept :
        _sync_timer{sync_timer_config} {}

    /// @brief Get readonly reference to the current configuration
    [[nodiscard]] constexpr const Config &config() const noexcept {
        return _config;
    }

    /// @brief Get mutable reference to the current configuration
    [[nodiscard]] Config &config() noexcept {
        return _config;
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

        // init is idempotent
        if (_nvs_entry.init().isError()) {
            logger.error("NVS init failed");
        }

        if (_load_requested) {
            _load_requested = false;

            logger.info("Loading config from NVS...");

            if (_nvs_entry.load(view()).isOk()) {
                _stored_crc = crc();
                logger.info(LogString::formatted("Config loaded from NVS (CRC: %u)", _stored_crc).view());

                if (not _config.isLatestVersion()) {
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

            _config = djc::Config::defaults();
        }

        if (const auto current_crc = crc(); current_crc != _stored_crc) {
            logger.info(LogString::formatted("Config changed, saving (CRC: %u -> %u)...", _stored_crc, current_crc).view());

            if (_nvs_entry.dump(view()).isOk() and _nvs_entry.commit().isOk()) {
                _stored_crc = current_crc;
                logger.info("Config saved, CRC updated");
            } else {
                logger.error("Config save failed");
            }
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigService")};
    
    Config _config{djc::Config::defaults()};
    memory::NVS _nvs_entry{"djc"};
    kf::math::Timer _sync_timer;
    kf::u32 _stored_crc{};
    bool _load_requested{false}, _reset_requested{false};

    [[nodiscard]] kf::Slice<kf::u8> view() noexcept {
        return {reinterpret_cast<kf::u8 *>(&_config), sizeof(Config)};
    }

    [[nodiscard]] kf::Slice<const kf::u8> view() const noexcept {
        return const_cast<ConfigService *>(this)->view();
    }

    [[nodiscard]] kf::u32 crc() const noexcept {
        return djc::math::crc32(view());
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
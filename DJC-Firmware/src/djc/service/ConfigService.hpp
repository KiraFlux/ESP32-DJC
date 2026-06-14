// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/math/Timer.hpp>
#include <kf/memory/Storage.hpp>

#include "djc/Config.hpp"
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

    /// @brief Requests an deferred save of the current config to NVS
    void requestSave() noexcept {
        _save_requested = true;
        logger.debug("save requested");
    }

    /// @brief Requests an deferred load of the config from NVS
    void requestLoad() noexcept {
        _load_requested = true;
        logger.debug("load requested");
    }

    /// @brief Requests an deferred reset of the config to defaults
    void requestReset() noexcept {
        _reset_requested = true;
        logger.debug("reset requested");
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigService")};

    static constexpr kf::math::Timer::Config sync_timer_config{
        .period = 5'000,
    };

    kf::memory::Storage<Config> _storage{
        .key = "DC",
        .config = djc::Config::defaults(),
    };

    kf::math::Timer _sync_timer{sync_timer_config};

    bool _save_requested{false}, _load_requested{false}, _reset_requested{false};

    KF_IMPL_TIMED_POLLABLE(ConfigService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (not _sync_timer.expired(now)) { return; }
        _sync_timer.start(now);

        if (_load_requested) {
            _load_requested = false;

            logger.info("Loading config from NVS");

            if (not _storage.load()) {
                logger.error("Failed to load config");
                requestReset();
                requestSave();
            }

            if (not _storage.config.isLatestVersion()) {
                logger.error("Config version is outdated");
                requestReset();
                requestSave();
            }
        }

        if (_reset_requested) {
            _reset_requested = false;

            logger.info("Resetting RAM config cache to defaults");
            _storage.config = djc::Config::defaults();
        }

        if (_save_requested) {
            _save_requested = false;

            logger.info("Saving config to NVS");

            if (not _storage.save()) {
                logger.error("Failed to save config into NVS");
            }
        }
    }
};

}// namespace djc::service
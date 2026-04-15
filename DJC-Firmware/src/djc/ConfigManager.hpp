// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/mixin/Singleton.hpp>

#include "djc/Config.hpp"

namespace djc {

struct ConfigManager final : kf::mixin::Singleton<ConfigManager> {

    [[nodiscard]] constexpr const Config &config() const noexcept { return _storage.config; }

    [[nodiscard]] Config &config() noexcept { return _storage.config; }

    [[nodiscard]] bool modified() const noexcept { return _modified; }

    void modified(bool is_modified) noexcept { _modified = is_modified; }

    void save() noexcept {
        logger.info("Saving config to NVS");

        if (not _storage.save()) {
            logger.error("Failed to save config into NVS");
        }
    }

    void load() noexcept {
        logger.info("Loading config from NVS");

        if (not _storage.load()) {
            logger.error("Failed to load config");
            reset();
            save();
        }

        if (not _storage.config.isLatestVersion()) {
            logger.error("Config version is outdated");
            reset();
            save();
        }
    }

    void reset() noexcept {
        logger.info("Resetting RAM config cache to defaults");

        _storage.config = djc::Config::defaults();
        modified(true);
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigManager")};

    kf::memory::Storage<Config> _storage{
        .key = "DC",
        .config = djc::Config::defaults(),
    };

    bool _modified{false};
};

}// namespace djc
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/Logger.hpp>
#include <kf/memory/Storage.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/Config.hpp"

namespace djc {

struct ConfigManager final : kf::mixin::NonCopyable {

    [[nodiscard]] const Config &config() const noexcept { return _storage.config; }

    [[nodiscard]] Config &config() noexcept { return _storage.config; }

    void save() noexcept {
        logger.info("Saving config to NVS");

        if (not _storage.save()) {
            logger.error("Failed to save config into NVS");
        }
    }

    void load() noexcept {
        logger.info("Loading config from NVS");

        if (not _storage.load()) {
            logger.warn("Failed to load config. Using defaults");
            _storage.config = Config::defaults();
            save();
        }
    }

    void reset() noexcept {
        logger.info("Set RAM config to defaults");

        _storage.config = djc::Config::defaults();
        save();
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigManager")};

    kf::memory::Storage<Config> _storage{
        .key = "DC",
        .config = djc::Config::defaults(),
    };
};

}// namespace djc
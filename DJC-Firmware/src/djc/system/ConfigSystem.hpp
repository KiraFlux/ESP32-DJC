// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/Config.hpp"
#include "djc/mixin/ServiceOwner.hpp"
#include "djc/service/ConfigService.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System owning the config service, handling deferred NVS operations
/// @note Wraps ConfigService, load on init, and polls it periodically
struct ConfigSystem :

    System<ConfigSystem, void()>,
    mixin::ServiceOwner<service::ConfigService>

{
    explicit ConfigSystem() noexcept :
        mixin::ServiceOwner<service::ConfigService>{service::ConfigService{
            sync_timer_config,
            {reinterpret_cast<kf::u8 *>(&_config), sizeof(Config)},
        }} {}

    /// @brief Get readonly reference to the current configuration
    [[nodiscard]] constexpr const Config &config() const noexcept {
        return _config;
    }

    /// @brief Get mutable reference to the current configuration
    [[nodiscard]] Config &config() noexcept {
        return _config;
    }

private:
    Config _config{djc::Config::defaults()};

    static constexpr kf::math::Timer::Config sync_timer_config{
        .period = 10'000,
    };

    KF_IMPL_INITABLE(ConfigSystem, void());
    void initImpl() noexcept {
        this->service().resettingStrategy([](kf::Slice<kf::u8> view) {
            auto &config = *reinterpret_cast<Config *>(view.data());

            config = Config::defaults();
        });

        this->service().onLoad([this](kf::Slice<kf::u8> view) {
            auto &config = *reinterpret_cast<Config *>(view.data());

            if (not _config.isLatestVersion()) {
                this->service().requestReset();
            }
        });

        this->service().requestLoad();
        this->service().sync();
    }

    KF_IMPL_TIMED_POLLABLE(ConfigSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
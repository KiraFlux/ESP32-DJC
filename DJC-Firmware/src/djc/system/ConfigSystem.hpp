// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "djc/config/DeviceConfig.hpp"
#include "djc/config/UserConfig.hpp"
#include "djc/mixin/ServiceOwner.hpp"
#include "djc/service/ConfigService.hpp"
#include "djc/system/System.hpp"

namespace djc::system {

/// @brief System owning the config service, handling deferred NVS operations
/// @tparam I Config Implementation (Must inherit from `::djc::config::ConfigTag`)
/// @note Wraps ConfigService, load on init, and polls it periodically
template<typename I> struct ConfigSystem :

    System<ConfigSystem, void()>,
    mixin::ServiceOwner<service::ConfigService>

{
    KF_CHECK_IMPL(I, ::djc::config::ConfigTag);
    using ConfigImpl = I;

    explicit ConfigSystem(const kf::math::Timer::Config &sync_timer_config) noexcept :
        mixin::ServiceOwner<service::ConfigService>{service::ConfigService{sync_timer_config, _config.view()}} {}

    /// @brief Get readonly reference to the current configuration
    [[nodiscard]] constexpr const ConfigImpl &config() const noexcept {
        return _config;
    }

    /// @brief Get mutable reference to the current configuration
    [[nodiscard]] ConfigImpl &config() noexcept {
        return _config;
    }

private:
    ConfigImpl _config{ConfigImpl::defaults()};

    KF_IMPL_INITABLE(ConfigSystem, void());
    void initImpl() noexcept {
        this->service().resettingStrategy([](kf::Slice<kf::u8> view) {
            if (auto c = ConfigImpl::interpret(view); c.isSome()) {
                c.unwrap().reset();
            }
        });

        this->service().onLoad([this](kf::Slice<kf::u8> view) {
            if (auto c = ConfigImpl::interpret(view); c.isSome()) {
                if (not c.unwrap().isLatest()) {
                    this->service().requestReset();
                }
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
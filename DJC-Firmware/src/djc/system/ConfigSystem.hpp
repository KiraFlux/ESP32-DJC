// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

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
        mixin::ServiceOwner<service::ConfigService>{{}} {}

private:
    KF_IMPL_INITABLE(ConfigSystem, void());
    void initImpl() noexcept {
        this->service().requestLoad();
        this->service().sync();
    }

    KF_IMPL_TIMED_POLLABLE(ConfigSystem);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        this->service().poll(now);
    }
};

}// namespace djc::system
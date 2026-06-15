// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <utility>

#include <kf/meta/CRTP.hpp>

#include "djc/service/Service.hpp"

namespace djc::mixin {

struct ServiceOwnerTag {};

/// @brief Mixin represents that class owns one service
template<typename ServiceType> struct ServiceOwner : ServiceOwnerTag {
    KF_CHECK_IMPL(ServiceType, ::djc::service::ServiceTag);

    explicit ServiceOwner(ServiceType &&service) noexcept :
        _service{std::move(service)} {}

    /// @brief Get mutable access to the service
    ServiceType &service() noexcept {
        return _service;
    }

    /// @brief Get readonly access to the service
    constexpr const ServiceType &service() const noexcept {
        return _service;
    }

private:
    ServiceType _service;
};

}// namespace djc::mixin
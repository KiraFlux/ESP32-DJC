// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <utility>

namespace djc::mixin {

/// @brief CRTP mixin that adds an `init(Args...)` method forwarding arguments to `initImpl(Args...)`
/// @tparam Impl     Derived class (must implement `initImpl(Args...)`)
template<typename Impl, typename Signature> struct Initable;

template<typename Impl, typename R, typename... Args> struct Initable<Impl, R(Args...)> {

    /// @brief Initializes the object with given arguments
    /// @param args Arguments forwarded to `initImpl`
    /// @return init result
    [[nodiscard]] R init(Args... args) noexcept {
        return static_cast<Impl *>(this)->initImpl(std::forward<Args>(args)...);
    }
};

template<typename Impl, typename... Args> struct Initable<Impl, void(Args...)> {

    /// @brief Initializes the object with given arguments
    /// @param args Arguments forwarded to `initImpl`
    void init(Args... args) noexcept {
        static_cast<Impl *>(this)->initImpl(std::forward<Args>(args)...);
    }
};

}// namespace djc::mixin

#define DJC_IMPL_INITABLE(__impl__, ...) friend struct ::djc::mixin::Initable<__impl__, __VA_ARGS__>
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/mixin/Resettable.hpp>
#include <kf/primitives.hpp>

#include "djc/math.hpp"

namespace djc::config {

/// @brief Persistent configuration structure
template<typename Impl, kf::u8 latest_version> struct Config : kf::mixin::Resettable<Impl> {

    kf::u8 version;

    [[nodiscard]] bool isLatest() const noexcept {
        return version == latest_version;
    }

    [[nodiscard]] constexpr auto view() noexcept -> kf::Slice<kf::u8> {
        return {
            reinterpret_cast<kf::u8 *>(this),
            sizeof(Impl),
        };
    }

    [[nodiscard]] constexpr auto view() const noexcept -> kf::Slice<const kf::u8> {
        return const_cast<Impl *>(this)->view();
    }

    [[nodiscard]] kf::u32 crc32() const noexcept {
        return math::crc32(view());
    }

    [[nodiscard]] static constexpr auto defaults() noexcept {
        Impl ret;

        ret.version = latest_version;
        ret.reset();

        return ret;
    }
};

}// namespace djc::config
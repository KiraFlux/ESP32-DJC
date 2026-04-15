// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/memory/Array.hpp>

namespace djc::memory {

template<typename T, typename Index, Index N> struct Box {
    kf::memory::Array<T, N> items;
    Index items_saved, selected_index;// 0..N

    constexpr Index maxItems() const noexcept { return N; }

    const T &selected() const noexcept { return items[selected_index]; }

    bool add(const T &item) noexcept {
        if (items_saved >= maxItems()) { return false; }

        items[items_saved] = item;
        items_saved += 1;

        return true;
    }

    static constexpr Box defaults() noexcept {
        return Box{
            .items = {},
            .items_saved = 0,
            .selected_index = 0,
        };
    }
};

}// namespace djc::memory

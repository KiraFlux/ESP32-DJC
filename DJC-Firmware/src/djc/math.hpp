// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Slice.hpp>
#include <kf/primitives.hpp>

namespace djc::math {

/// @brief CRC-32 (IEEE 802.3) checksum
[[nodiscard]] kf::u32 crc32(kf::Slice<const kf::u8> data) {
    constexpr auto reflected_polynomial{0xEDB88320u};

    auto crc = static_cast<kf::u32>(-1);

    for (auto byte: data) {
        crc ^= byte;

        for (auto j = 0u; j < 8; j += 1) {
            if (crc & 1) {
                crc = (crc >> 1) ^ reflected_polynomial;
            } else {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

}// namespace djc::math
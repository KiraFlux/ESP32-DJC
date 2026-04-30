// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/aliases.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Callbacked.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::protocol {

/// @brief Raw protocol – sends ManualInput as a binary blob and invokes a callback on received data
/// @note
/// The protocol simply sends the `ManualInput` struct verbatim.
/// Incoming data is forwarded directly to the callback as is`.
struct RawProtocol : Protocol, kf::mixin::Callbacked<kf::memory::Slice<const kf::u8>> {

    void poll(kf::math::Milliseconds, const ManualInput &input, transport::TransportLink &transport_link) noexcept override {
        (void) transport_link.send({reinterpret_cast<const kf::u8 *>(&input), sizeof(ManualInput)});
    }

    void receive(kf::memory::Slice<const kf::u8> buffer) noexcept override {
        this->invoke(buffer);
    }
};

}// namespace djc::protocol
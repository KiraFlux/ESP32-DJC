// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>

#include <kf/aliases.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::protocol {

struct ProtocolLink : kf::mixin::NonCopyable {

    void protocol(Protocol &new_protocol) noexcept {
        _protocol = &new_protocol;
    }

    void poll(kf::math::Milliseconds now, const ManualInput &input, transport::TransportLink &transport_link) noexcept {
        if (nullptr == _protocol) {
            logger.error("poll: no protocol set");
            return;
        }

        _protocol->poll(now, input, transport_link);
    }

    void receive(kf::memory::Slice<const kf::u8> buffer) noexcept {
        if (nullptr == _protocol) {
            logger.error("receive: no protocol set");
            return;
        }

        _protocol->receive(buffer);
    }

private:
    static constexpr auto logger{kf::Logger::create("ProtocolLink")};

    Protocol *_protocol{nullptr};
};

}// namespace djc::protocol
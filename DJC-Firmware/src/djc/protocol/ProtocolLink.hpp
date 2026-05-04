// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Logger.hpp>

#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::protocol {

namespace internal {

struct ProtocolLinkConfig {

    kf::math::Milliseconds poll_period;

    [[nodiscard]] static constexpr ProtocolLinkConfig defaults() noexcept {
        return ProtocolLinkConfig{
            .poll_period = static_cast<kf::math::Milliseconds>(1000 / 50),
        };
    }
};

}// namespace internal

struct ProtocolLink :

    kf::mixin::NonCopyable,
    kf::mixin::Configurable<internal::ProtocolLinkConfig>

{
    using Config = internal::ProtocolLinkConfig;

    using kf::mixin::Configurable<Config>::Configurable;

    void protocol(Protocol &new_protocol) noexcept {
        _protocol = &new_protocol;
    }

    void poll(kf::math::Milliseconds now, const ManualInput &input, transport::TransportLink &transport_link) noexcept {
        if (nullptr == _protocol) {
            logger.error("poll: no protocol set");
            return;
        }

        if (_poll_timer.expired(now) or _poll_timer_reset_required) {
            _poll_timer.start(now);
            _poll_timer_reset_required = false;

            _protocol->poll(now, input, transport_link);
        }
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
    kf::math::Timer _poll_timer{this->config().poll_period};
    bool _poll_timer_reset_required{true};
};

}// namespace djc::protocol
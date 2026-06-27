// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/Resettable.hpp>
#include <kf/primitives.hpp>

#include "djc/service/Service.hpp"
#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::internal {

/// @brief Configuration parameters for the PeerScanningService service
struct PeerScannerConfig : kf::mixin::Resettable<PeerScannerConfig> {

    /// @brief How long an entry stays in the list without being refreshed
    kf::math::Milliseconds entry_max_life_time;

    /// @brief Interval between periodic clean-ups and list compaction
    kf::math::Timer::Config entries_list_update_timer;

private:
    KF_IMPL_RESETTABLE(PeerScannerConfig);
    void resetImpl() noexcept {
        entry_max_life_time = 8'000;
        entries_list_update_timer.period = 100;
    }
};

}// namespace djc::internal

namespace djc::service {

/// @brief Background service that listens for foreign (broadcast) packets and maintains a list of visible peers
/// @note
/// The scanner subscribes to `TransportLink::onReceiveForeign` and keeps up to `max_entries` entries
/// Entries are refreshed every time a packet from the corresponding peer is received.
/// Periodically, expired entries are removed and the list is compacted so that the first `peers().size()` elements are always valid.
struct PeerScanningService :

    Service<PeerScanningService>,
    kf::mixin::Initable<PeerScanningService, void()>,
    kf::mixin::Configurable<internal::PeerScannerConfig>

{
    using Config = internal::PeerScannerConfig;

    /// @brief A single entry in the peer list.
    struct Entry final {
        transport::PeerAddress address;  ///< Address of the peer.
        kf::math::Milliseconds last_seen;///< Timestamp of the last received packet (millis).
    };

    /// @brief Maximum number of peers the scanner can remember simultaneously
    static constexpr auto max_entries{8};

    explicit constexpr PeerScanningService(const Config &config, transport::TransportLink &transport_link) noexcept :
        Configurable<Config>{config}, _transport_link{transport_link} {}

    /// @brief Returns a slice of the currently active peer entries.
    /// @return A contiguous view of the first `_active_count` elements of the internal array.
    /// @note The slice is valid only until the next call to `poll()`.
    ///       The entries are sorted in order of registration (oldest first).
    [[nodiscard]] kf::Slice<const kf::TrivialOption<Entry>> peers() const noexcept {
        return {_entries.data(), _active_count};
    }

private:
    kf::memory::Array<kf::TrivialOption<Entry>, max_entries> _entries{};
    kf::math::Timer _update_poll_timer{this->config().entries_list_update_timer};
    transport::TransportLink &_transport_link;
    kf::math::Milliseconds _last_poll_time{0};
    kf::usize _active_count{0};

    KF_IMPL_INITABLE(PeerScanningService, void());
    void initImpl() noexcept {
        _update_poll_timer.start(0);// enable timer

        _transport_link.onReceiveForeign([this](const transport::PeerAddress &address, kf::Slice<const kf::u8> buffer) -> void {
            // search for mathing entry
            for (auto &entry: _entries) {
                if (entry.isSome() and entry.unwrap().address == address) {
                    entry.unwrap().last_seen = _last_poll_time;
                    return;
                }
            }

            // search for first empty entry
            for (auto &entry: _entries) {
                if (entry.isNone()) {
                    entry = kf::someTrivial(Entry{
                        .address = address,
                        .last_seen = _last_poll_time,
                    });
                    return;
                }
            }

            // no available entries -> replace oldest with newest
            kf::usize oldest_entry_index{0};
            for (auto i = 1u; i < max_entries; i += 1) {
                if (_entries[i].unwrap().last_seen < _entries[oldest_entry_index].unwrap().last_seen) {
                    oldest_entry_index = i;
                }
            }
            _entries[oldest_entry_index] = kf::someTrivial(Entry{address, _last_poll_time});
        });
    }

    KF_IMPL_TIMED_POLLABLE(PeerScanningService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _last_poll_time = now;

        if (_update_poll_timer.expired(now)) {
            _update_poll_timer.start(now);

            if (_transport_link.connected()) {
                const auto &active_address = _transport_link.activePeerAddress().unwrap();
                for (auto &entry: _entries) {
                    if (entry.isSome() and entry.unwrap().address == active_address) {
                        entry.reset();
                        break;
                    }
                }
            }

            auto write_index = 0u;
            for (auto read_index = 0u; read_index < max_entries; read_index += 1) {
                if (_entries[read_index].isSome()) {
                    if (now > _entries[read_index].unwrap().last_seen + this->config().entry_max_life_time) {
                        _entries[read_index].reset();
                    } else {
                        if (write_index != read_index) {
                            _entries[write_index] = _entries[read_index];
                        }
                        write_index += 1;
                    }
                }
            }

            for (auto i = write_index; i < max_entries; i += 1) {
                _entries[i].reset();
            }
            _active_count = write_index;
        }
    }
};

}// namespace djc::service
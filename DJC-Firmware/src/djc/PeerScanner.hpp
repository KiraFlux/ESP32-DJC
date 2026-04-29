// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Configurable.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/transport/PeerAddress.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc {

namespace internal {

/// @brief Configuration parameters for the PeerScanner service.
struct PeerScannerConfig final : kf::mixin::NonCopyable {
    kf::math::Milliseconds
        entry_max_life_time,       ///< How long an entry stays in the list without being refreshed.
        entries_list_update_period;///< Interval between periodic clean-ups and list compaction.
};

}// namespace internal

/// @brief Background service that listens for foreign (broadcast) packets and maintains a list of visible peers.
/// @note
/// The scanner subscribes to `TransportLink::onReceiveForeign` and keeps up to `max_entries` entries
/// Entries are refreshed every time a packet from the corresponding peer is received.
/// Periodically, expired entries are removed and the list is compacted so that the first `peers().size()` elements are always valid.
struct PeerScanner final :

    kf::mixin::NonCopyable,
    kf::mixin::Initable<PeerScanner, void>,
    kf::mixin::Configurable<internal::PeerScannerConfig>,
    kf::mixin::TimedPollable<PeerScanner>

{
    using Config = internal::PeerScannerConfig;

    /// @brief A single entry in the peer list.
    struct Entry final {
        transport::PeerAddress address;  ///< Address of the peer.
        kf::math::Milliseconds last_seen;///< Timestamp of the last received packet (millis).
    };

    /// @brief Maximum number of peers the scanner can remember simultaneously
    static constexpr auto max_entries{8};

    explicit constexpr PeerScanner(const Config &config, transport::TransportLink &transport_link) noexcept :
        Configurable<Config>{config}, _transport_link{transport_link} {}

    /// @brief Returns a slice of the currently active peer entries.
    /// @return A contiguous view of the first `_active_count` elements of the internal array.
    /// @note The slice is valid only until the next call to `poll()`.
    ///       The entries are sorted in order of registration (oldest first).
    [[nodiscard]] kf::memory::Slice<const kf::Option<Entry>> peers() const noexcept {
        return {_entries.data(), _active_count};
    }

private:
    kf::memory::Array<kf::Option<Entry>, max_entries> _entries{};
    kf::math::Timer _update_poll_timer{this->config().entries_list_update_period};
    transport::TransportLink &_transport_link;
    kf::math::Milliseconds _last_poll_time{0};
    kf::usize _active_count{0};

    // impl
    using This = PeerScanner;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        _update_poll_timer.start(0);// enable timer

        _transport_link.onReceiveForeign([this](const transport::PeerAddress &address, kf::memory::Slice<const kf::u8> buffer) -> void {
            // search for mathing entry
            for (auto &entry: _entries) {
                if (entry.hasValue() and entry.value().address == address) {
                    entry.value().last_seen = _last_poll_time;
                    return;
                }
            }

            // search for first empty entry
            for (auto &entry: _entries) {
                if (not entry.hasValue()) {
                    entry = Entry{address, _last_poll_time};
                    return;
                }
            }

            // no available entries -> replace oldest with newest
            kf::usize oldest_entry_index{0};
            for (auto i = 1u; i < max_entries; i += 1) {
                if (_entries[i].value().last_seen < _entries[oldest_entry_index].value().last_seen) {
                    oldest_entry_index = i;
                }
            }
            _entries[oldest_entry_index] = Entry{address, _last_poll_time};
        });
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        _last_poll_time = now;

        if (_update_poll_timer.expired(now)) {
            _update_poll_timer.start(now);

            auto write_index = 0u;
            for (auto read_index = 0u; read_index < max_entries; read_index += 1) {
                if (_entries[read_index].hasValue() and (now > _entries[read_index].value().last_seen + this->config().entry_max_life_time)) {
                    _entries[read_index] = {};
                } else {
                    if (write_index != read_index) {
                        _entries[write_index] = _entries[read_index];
                    }
                    write_index += 1;
                }
            }

            for (auto i = write_index; i < max_entries; i += 1) {
                _entries[i] = {};
            }
            _active_count = write_index;
        }
    }
};

}// namespace djc
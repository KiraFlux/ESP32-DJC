// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/Range.hpp>
#include <kf/aliases.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>

#include "djc/transport/PeerAddress.hpp"

namespace djc {

/// @brief Registry of favorite peers (bookmarks).
///
/// Maintains a list of peer entries inside an externally‑provided array of slots.
/// The registry does not own the memory, only manipulates it.
///
/// @note All methods are safe to call from different UI callbacks as long as the underlying storage is exclusively owned by the registry.
struct PeerFavoritesRegistry final : kf::mixin::NonCopyable {

    /// @brief A single favorite‑peer record.
    struct Entry final {
        using TrustType = kf::u8;

        static constexpr kf::Range<TrustType> trust_range{.start = 0, .end = 10};

        transport::PeerAddress address;         ///< Peer address.
        TrustType trust;                        ///< Trust priority (0 - no trust, 1.. - auto connect)
        kf::memory::Array<char, 16> description;///< Human‑readable description.

        /// @brief Factory method for a new, empty‑description entry.
        [[nodiscard]] static Entry create(const transport::PeerAddress &address) noexcept {
            return Entry{
                .address = address,
                .trust = trust_range.start,
                .description = {},
            };
        }
    };

    explicit constexpr PeerFavoritesRegistry(kf::memory::Slice<kf::Option<Entry>> entries) noexcept :
        _entries{entries} {}

    /// @brief Return the entire slot array, including empty slots.
    [[nodiscard]] kf::memory::Slice<const kf::Option<Entry>> all() const noexcept { return _entries; }

    /// @brief Obtain a const pointer to an entry by address.
    /// @param address Peer address to search for.
    [[nodiscard]] const kf::Option<Entry> &get(const transport::PeerAddress &address) const noexcept {
        static constexpr kf::Option<Entry> none{};

        if (const auto index = indexOf(address); index.hasValue()) {
            if (const auto &option = _entries[index.value()]; option.hasValue()) { return option; }
        }

        return none;
    }

    /// @brief Check exists to an entry by address
    [[nodiscard]] bool exists(const transport::PeerAddress &address) const noexcept {
        return indexOf(address).hasValue();
    }

    /// @brief Update already existed or Add a new entry
    /// @return true, or false if the list is full.
    [[nodiscard]] bool put(const Entry &entry_to_add) noexcept {
        if (const auto same_address_entry_index = indexOf(entry_to_add.address); same_address_entry_index.hasValue()) {
            _entries[same_address_entry_index.value()].value(entry_to_add);
            return true;
        }

        for (auto &entry: _entries) {
            if (not entry.hasValue()) {
                entry.value(entry_to_add);
                return true;
            }
        }

        return false;
    }

    /// @brief Remove an entry by address.
    /// @param address Peer address to remove.
    /// @return true if an entry was actually removed.
    [[nodiscard]] bool remove(const transport::PeerAddress &address) noexcept {
        const auto index = indexOf(address);
        if (index.hasValue()) {
            _entries[index.value()] = {};
        }

        return index.hasValue();
    }

private:
    kf::memory::Slice<kf::Option<Entry>> _entries;

    /// @brief Find the index of an entry by address.
    /// @param address Peer address to search for.
    /// @return Option containing the index, or an empty option if not found.
    [[nodiscard]] kf::Option<kf::usize> indexOf(const transport::PeerAddress &address) const noexcept {
        for (auto index = 0u; index < _entries.size(); index += 1) {
            const auto &item = _entries[index];
            if (item.hasValue() and item.value().address == address) {
                return {index};
            }
        }
        return {};
    }
};

}// namespace djc
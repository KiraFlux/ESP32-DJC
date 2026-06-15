// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
#include <kf/Range.hpp>
#include <kf/Slice.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/primitives.hpp>

#include "djc/transport/PeerAddress.hpp"

namespace djc {

/// @brief Registry of favorite peers
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

        transport::PeerAddress address;  ///< Peer address.
        TrustType trust;                 ///< Trust priority (0 - no trust, 1.. - auto connect)
        kf::memory::Array<char, 16> name;///< Human‑readable description.

        /// @brief Factory method for a new, empty‑description entry.
        [[nodiscard]] static Entry create(const transport::PeerAddress &address) noexcept {
            return Entry{
                .address = address,
                .trust = trust_range.start,
                .name = {"New-Peer"},
            };
        }
    };

    /// @brief Set entries source
    void entries(kf::Slice<kf::TrivialOption<Entry>> new_entries) noexcept {
        _entries = new_entries;

        _active_count = 0;
        for (const auto &entry: _entries) {
            _active_count += static_cast<kf::usize>(entry.isSome());
        }
    }

    /// @brief Return all non-empty entries
    [[nodiscard]] auto all() const noexcept -> kf::Slice<const kf::TrivialOption<Entry>> {
        return {_entries.data(), _active_count};
    }

    /// @brief Obtain a const pointer to an entry by address.
    /// @param address Peer address to search for.
    [[nodiscard]] auto get(const transport::PeerAddress &address) const noexcept -> kf::Option<const Entry &> {
        if (const auto index = indexOf(address); index.isSome()) {
            if (const auto &option = _entries[index.unwrap()]; option.isSome()) {
                return kf::someRef(option.unwrap());
            }
        }

        return kf::none;
    }

    /// @brief Check exists to an entry by address
    [[nodiscard]] bool exists(const transport::PeerAddress &address) const noexcept {
        return indexOf(address).isSome();
    }

    /// @brief Update already existed or Add a new entry
    /// @return true, or false if the list is full.
    [[nodiscard]] bool put(const Entry &entry_to_add) noexcept {
        if (auto index = indexOf(entry_to_add.address); index.isSome()) {
            _entries[index.unwrap()] = kf::someTrivial(entry_to_add);
            return true;
        }

        const bool can_add = (_active_count < _entries.size());
        if (can_add) {
            _entries[_active_count] = kf::someTrivial(entry_to_add);
            _active_count += 1;
        }

        return can_add;
    }

    /// @brief Remove an entry by address.
    /// @param address Peer address to remove.
    /// @return true if an entry was actually removed.
    [[nodiscard]] bool remove(const transport::PeerAddress &address) noexcept {
        const auto index = indexOf(address);
        const auto last_index = _active_count - 1u;

        if (index.isNone()) { return false; }

        if (index.unwrap() != last_index) {
            _entries[index.unwrap()] = _entries[last_index];
        }

        _entries[last_index] = {};
        _active_count -= 1;

        return true;
    }

private:
    kf::Slice<kf::TrivialOption<Entry>> _entries{};
    kf::usize _active_count{0};

    /// @brief Find the index of an entry by address.
    /// @param address Peer address to search for.
    /// @return Option containing the index, or an empty option if not found.
    [[nodiscard]] kf::Option<kf::usize> indexOf(const transport::PeerAddress &address) const noexcept {
        for (auto index = 0u; index < _entries.size(); index += 1) {
            const auto &item = _entries[index];
            if (item.isSome() and item.unwrap().address == address) {
                return kf::some(index);
            }
        }
        return kf::none;
    }
};

}// namespace djc
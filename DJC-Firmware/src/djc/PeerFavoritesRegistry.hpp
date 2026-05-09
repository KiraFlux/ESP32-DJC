// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/Option.hpp>
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
    struct Entry final : kf::mixin::NonCopyable {
        transport::PeerAddress address;         ///< Peer address.
        bool trusted;                           ///< Trust flag (reserved for future use).
        kf::memory::Array<char, 16> description;///< Human‑readable description.

        /// @brief Factory method for a new, empty‑description entry.
        [[nodiscard]] static Entry create(const transport::PeerAddress &address) noexcept {
            return Entry{
                .address = address,
                .trusted = false,
                .description = {},
            };
        }
    };

    explicit constexpr PeerFavoritesRegistry(kf::memory::Slice<kf::Option<Entry>> entries) noexcept :
        _entries{entries} {}

    /// @brief Return the entire slot array, including empty slots.
    [[nodiscard]] kf::memory::Slice<const kf::Option<Entry>> all() const noexcept { return _entries; }

    /// @brief Obtain a mutable pointer to an entry by address.
    /// @param address Peer address to search for.
    /// @return Pointer to the entry, or nullptr if not found.
    // todo replace with Option<Entry &> (not exists yet..)
    [[nodiscard]] Entry *get(const transport::PeerAddress &address) noexcept {
        const auto index = indexOf(address);
        if (index.hasValue()) {
            auto &option = _entries[index.value()];
            if (option.hasValue()) { return &option.value(); }
        }

        return nullptr;
    }

    /// @brief Obtain a read‑only pointer to an entry by address.
    /// @param address Peer address to search for.
    /// @return Pointer to a const entry, or nullptr if not found.
    [[nodiscard]] const Entry *get(const transport::PeerAddress &address) const noexcept {
        return const_cast<PeerFavoritesRegistry *>(this)->get(address);
    }

    /// @brief Add a new entry.
    /// @param address Peer address to add.
    /// @return true if the entry was added, false if it already exists or the list is full.
    [[nodiscard]] bool add(const transport::PeerAddress &address) noexcept {
        const auto can_add = not indexOf(address).hasValue();

        if (can_add) {
            for (auto &entry: _entries) {
                if (not entry.hasValue()) {
                    entry.value(Entry::create(address));
                    break;
                }
            }
        }

        return can_add;
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
    kf::memory::Slice<kf::Option<Entry>> _entries;///< External slot array

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
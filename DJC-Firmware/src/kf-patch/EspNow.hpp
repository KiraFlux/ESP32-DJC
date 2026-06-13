// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// std
#include <algorithm>// std::copy
#include <utility>  // std::move

// esp
#include <esp_mac.h>
#include <esp_now.h>

// lib
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/io/Writable.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/MacAddressed.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/Singleton.hpp"
#include "kf/mixin/StringRepresentable.hpp"
#include "kf/network/MacAddress.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

#define EVAL_KIND(__esp_error__) (__esp_error__ - ESP_ERR_ESPNOW_BASE)

#define CASE_RETURN(__v) \
    case __v: return #__v

struct EspNowError final : mixin::StringRepresentable<EspNowError, memory::StringView> {

    enum Kind : u8 {
        UnknownError = EVAL_KIND(ESP_ERR_ESPNOW_BASE),      ///< Unknown ESP API error
        NotInitialized = EVAL_KIND(ESP_ERR_ESPNOW_NOT_INIT),///< ESP-NOW protocol not initialized
        InvalidArg = EVAL_KIND(ESP_ERR_ESPNOW_ARG),         ///< Invalid argument passed to API
        NoMemory = EVAL_KIND(ESP_ERR_ESPNOW_NO_MEM),        ///< Insufficient memory for peer addition
        PeerListIsFull = EVAL_KIND(ESP_ERR_ESPNOW_FULL),    ///< Peer list is at maximum capacity
        PeerNotFound = EVAL_KIND(ESP_ERR_ESPNOW_NOT_FOUND), ///< Peer not found in peer list
        InternalError = EVAL_KIND(ESP_ERR_ESPNOW_INTERNAL), ///< ESP-NOW internal API error
        PeerAlreadyExists = EVAL_KIND(ESP_ERR_ESPNOW_EXIST),///< Peer already exists in list
        IncorrectWiFiMode = EVAL_KIND(ESP_ERR_ESPNOW_IF),   ///< Incorrect WiFi interface mode set
        TooBigMessage,                                      ///< Message size exceeds ESP_NOW_MAX_DATA_LEN
    } kind;

    [[nodiscard]] static constexpr auto create(Kind kind) noexcept -> ErrorWrapper<EspNowError> {
        return error(EspNowError{.kind = kind});
    }

    [[nodiscard]] static constexpr auto fromEspErr(esp_err_t e) noexcept -> ErrorWrapper<EspNowError> {
        return create((ESP_ERR_ESPNOW_BASE <= e and e <= ESP_ERR_ESPNOW_IF) ? UnknownError : static_cast<Kind>(e - ESP_ERR_ESPNOW_BASE));
    }

private:
    KF_IMPL_STRING_REPRESENTABLE(EspNowError, memory::StringView);
    memory::StringView toStringImpl() const noexcept {

        switch (kind) {
            CASE_RETURN(EspNowError::NotInitialized);
            CASE_RETURN(EspNowError::InvalidArg);
            CASE_RETURN(EspNowError::NoMemory);
            CASE_RETURN(EspNowError::PeerListIsFull);
            CASE_RETURN(EspNowError::PeerNotFound);
            CASE_RETURN(EspNowError::InternalError);
            CASE_RETURN(EspNowError::PeerAlreadyExists);
            CASE_RETURN(EspNowError::IncorrectWiFiMode);
            CASE_RETURN(EspNowError::TooBigMessage);
            default:
                CASE_RETURN(EspNowError::UnknownError);
        }
    }
};

#undef EVAL_KIND
#undef CASE_RETURN

}// namespace kf::internal

namespace kf::network {

/// @brief ESP-NOW protocol encapsulataion
/// @note Singleton wrapper for ESP-NOW API with peer management and callbacks
struct EspNow final :

    mixin::Singleton<EspNow>,
    mixin::MacAddressed,
    mixin::Initable<EspNow, Result<void, internal::EspNowError>>,
    mixin::Quitable<EspNow>,
    mixin::Callbacked<const MacAddress &, Slice<const u8>>

{
    /// @brief ESP-NOW operation error type
    using Error = internal::EspNowError;

    using VoidResult = Result<void, Error>;

    /// @brief ESP‑NOW peer representation with communication capabilities
    /// @note Manages a single peer: registration, sending
    /// @note Move‑only, non‑copyable
    struct Peer final :

        mixin::NonCopyable,
        mixin::MacAddressed,
        io::Writable<Peer, VoidResult>

    {
        /// @brief Peer configuration
        struct Config final {
            MacAddress mac_address;
            bool wifi_interface_sta;///< (true - STA, false - AP)
        };

        /// @brief Move constructor
        /// @param other Source peer (becomes inactive)
        Peer(Peer &&other) noexcept : mixin::MacAddressed{std::move(other)}, _owns{other._owns} {
            other._owns = false;
        }

        /// @brief Destructor - automatically removes the peer if owned
        ~Peer() noexcept {
            if (_owns) { (void) del(); }
        }

        /// @brief Move assignment
        /// @param other Source peer (becomes inactive)
        /// @return Reference to this
        Peer &operator=(Peer &&other) noexcept {
            if (this != &other) {
                if (_owns) { (void) del(); }

                static_cast<mixin::MacAddressed &>(*this) = std::move(other);
                _owns = other._owns;
                other._owns = false;
            }
            return *this;
        }

        /// @brief Add a new peer to ESP‑NOW network
        /// @param config Peer Configuration
        /// @return Peer object on success, Error on failure
        [[nodiscard]] static auto create(const Config &config) noexcept -> Result<Peer, Error> {
            esp_now_peer_info_t peer_info{
                .channel = 0,
                .ifidx = static_cast<wifi_interface_t>(config.wifi_interface_sta ? WIFI_IF_STA : WIFI_IF_AP),
                .encrypt = false,
            };
            std::copy(config.mac_address.begin(), config.mac_address.end(), peer_info.peer_addr);

            const auto e = esp_now_add_peer(&peer_info);
            if (ESP_OK == e) {
                return ok(std::move(Peer{config.mac_address}));
            } else {
                return Error::fromEspErr(e);
            }
        }

        /// @brief Remove peer from ESP-NOW network
        [[nodiscard]] VoidResult del() noexcept {
            const auto e = esp_now_del_peer(_mac_address.data());
            if (ESP_OK == e) {
                return ok();
            } else {
                return Error::fromEspErr(e);
            }
        }

        /// @brief Check if peer exists in ESP-NOW network
        [[nodiscard]] bool exist() noexcept {
            return esp_now_is_peer_exist(_mac_address.data());
        }

    private:
        bool _owns{true};

        /// @note Called only by Peer::create
        explicit Peer(const MacAddress &mac) noexcept : mixin::MacAddressed{mac} {}

        /// @brief Internal send implementation
        [[nodiscard]] VoidResult send(const u8 *data, usize len) noexcept {
            const auto e = esp_now_send(_mac_address.data(), data, len);
            if (ESP_OK == e) {
                return ok();
            } else {
                return Error::fromEspErr(e);
            }
        }

        KF_IMPL_WRITABLE(Peer, VoidResult);

        VoidResult writeBufferImpl(Slice<const u8> buffer) noexcept {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) { return Error::create(Error::TooBigMessage); }
            return send(buffer.data(), buffer.size());
        }

        template<typename T> VoidResult writePacketImpl(T &&packet) noexcept {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return send(reinterpret_cast<const u8 *>(&packet), sizeof(T));
        }

        template<typename T> VoidResult writeMixedImpl(T &&header, Slice<const u8> buffer) noexcept {
            const auto mixed_size = sizeof(T) + buffer.size();
            if (mixed_size > ESP_NOW_MAX_DATA_LEN) { return Error::create(Error::TooBigMessage); }
            u8 mixed[mixed_size];

            const auto header_data = reinterpret_cast<const u8 *>(&header);
            std::copy(header_data, header_data + sizeof(T), mixed);
            std::copy(buffer.begin(), buffer.end(), mixed + sizeof(T));

            return send(mixed, mixed_size);
        }
    };

private:
    friend struct ::kf::mixin::Singleton<EspNow>;

    /// @brief Private constructor (singleton)
    constexpr EspNow() noexcept : MacAddressed{{}} {}

    /// @brief ESP‑NOW receive callback (static wrapper)
    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) noexcept {
        MacAddress source_mac_address;
        std::copy(raw_mac_address, raw_mac_address + ESP_NOW_ETH_ALEN, source_mac_address.begin());

        EspNow::instance().invoke(source_mac_address, Slice<const u8>{data, static_cast<usize>(size)});
    }

    KF_IMPL_INITABLE(EspNow, VoidResult);
    VoidResult initImpl() noexcept {
        esp_err_t e;

        if (e = esp_now_init(); ESP_OK != e) { goto fail; }
        if (e = esp_now_register_recv_cb(onReceive); ESP_OK != e) { goto fail; }

        (void) esp_read_mac(_mac_address.data(), ESP_MAC_WIFI_STA);

        return ok();
    fail:
        return Error::fromEspErr(e);
    }

    KF_IMPL_QUITABLE(EspNow);
    void quitImpl() noexcept {
        (void) esp_now_unregister_recv_cb();
        (void) esp_now_deinit();
    }
};

}// namespace kf::network
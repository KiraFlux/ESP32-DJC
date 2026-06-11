// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// std
#include <utility>

// esp
#include <esp_mac.h>
#include <esp_now.h>

// lib
#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/io/Writable.hpp"
#include "kf/memory/Array.hpp"
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

struct EspNowError final : mixin::StringRepresentable<EspNowError, memory::StringView> {

    enum Kind : u8 {
        InternalError,    ///< ESP-NOW internal API error
        UnknownError,     ///< Unknown ESP API error
        NotInitialized,   ///< ESP-NOW protocol not initialized
        IncorrectWiFiMode,///< Incorrect WiFi interface mode set
        PeerListIsFull,   ///< Peer list is at maximum capacity
        InvalidArg,       ///< Invalid argument passed to API
        NoMemory,         ///< Insufficient memory for peer addition
        PeerAlreadyExists,///< Peer already exists in list
        PeerNotFound,     ///< Peer not found in peer list
        TooBigMessage,    ///< Message size exceeds ESP_NOW_MAX_DATA_LEN
    } kind;

    static constexpr EspNowError create(Kind kind) noexcept {
        return EspNowError{.kind = kind};
    }

private:
    KF_IMPL_STRING_REPRESENTABLE(EspNowError, memory::StringView);
    memory::StringView toStringImpl() const noexcept {

#define CASE_RETURN(__v) \
    case __v: return #__v

        switch (kind) {
            CASE_RETURN(EspNowError::NotInitialized);
            CASE_RETURN(EspNowError::InternalError);
            CASE_RETURN(EspNowError::TooBigMessage);
            CASE_RETURN(EspNowError::InvalidArg);
            CASE_RETURN(EspNowError::NoMemory);
            CASE_RETURN(EspNowError::PeerNotFound);
            CASE_RETURN(EspNowError::IncorrectWiFiMode);
            CASE_RETURN(EspNowError::PeerListIsFull);
            CASE_RETURN(EspNowError::PeerAlreadyExists);
            default:
                CASE_RETURN(EspNowError::UnknownError);
        }

#undef CASE_RETURN
    }
};

}// namespace kf::internal

namespace kf::network {

/// @brief Encapsulates ESP-NOW protocol in safe C++ abstractions
/// @note Singleton wrapper for ESP-NOW API with peer management and callbacks
struct EspNow final :

    kf::mixin::Singleton<EspNow>,
    mixin::MacAddressed,
    kf::mixin::Initable<EspNow, Result<void, internal::EspNowError>>,
    kf::mixin::Quitable<EspNow>,
    kf::mixin::Callbacked<const MacAddress &, Slice<const u8>>

{
    /// @brief Maximum number of peers that can be stored in the registry
    static constexpr usize max_peers{20};

    /// @brief ESP-NOW operation error codes
    using Error = internal::EspNowError;

    /// @brief ESP‑NOW peer representation with communication capabilities
    /// @note Manages a single peer: registration, sending, and receive callback
    /// @note Move‑only, non‑copyable
    struct Peer final :

        mixin::NonCopyable,
        mixin::MacAddressed,
        io::Writable<Peer, kf::Result<void, Error>>

    {
        friend struct EspNow;

        /// @brief Default configuration (station interface)
        struct Config final {
            wifi_interface_t wifi_interface;///< WiFi interface (STA or AP)

            [[nodiscard]] static constexpr Config defaults() noexcept {
                return Config{
                    .wifi_interface = WIFI_IF_STA,
                };
            }
        };

        /// @brief Move constructor
        /// @param other Source peer (becomes inactive)
        Peer(Peer &&other) noexcept : mixin::MacAddressed{std::move(other)}, _owns{other._owns} {
            other._owns = false;
        }

        /// @brief Move assignment
        /// @param other Source peer (becomes inactive)
        /// @return Reference to this
        Peer &operator=(Peer &&other) noexcept {
            if (this != &other) {
                if (_owns) {
                    (void) del();
                }

                static_cast<mixin::MacAddressed &>(*this) = std::move(other);
                _owns = other._owns;
                other._owns = false;
            }

            return *this;
        }

        /// @brief Add a new peer to ESP‑NOW network
        /// @param mac MAC address of the peer
        /// @param config Configuration parameters (optional)
        /// @return Peer object on success, Error on failure
        /// @note Automatically registers the peer with the ESP‑NOW subsystem
        [[nodiscard]] static Result<Peer, Error> create(const MacAddress &mac, Config config = Config::defaults()) noexcept {
            esp_now_peer_info_t peer_info{
                .channel = 0,
                .ifidx = config.wifi_interface,
                .encrypt = false,
            };
            std::copy(mac.begin(), mac.end(), peer_info.peer_addr);

            const auto result = esp_now_add_peer(&peer_info);

            if (ESP_OK == result) {
                return ok(std::move(Peer{mac}));
            } else {
                return error(translateEspError(result));
            }
        }

        /// @brief Remove peer from ESP-NOW network
        /// @return Success or Error
        /// @note Also removes any associated receive handler
        [[nodiscard]] Result<void, Error> del() noexcept {
            const auto result = esp_now_del_peer(_mac_address.data());

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspError(result));
            }
        }

        /// @brief Check if peer exists in ESP-NOW network
        /// @return true if peer is registered with ESP-NOW
        [[nodiscard]] bool exist() noexcept {
            return esp_now_is_peer_exist(_mac_address.data());
        }

        /// @brief Destructor - automatically removes the peer if owned
        ~Peer() noexcept {
            if (_owns) {
                (void) del();
            }
        }

    private:
        using WriteResult = Result<void, Error>;

        bool _owns;

        /// @brief Private constructor – creates a peer owned by this object
        /// @param mac MAC address
        /// @note Called only by Peer::create
        explicit Peer(const MacAddress &mac) noexcept : mixin::MacAddressed{mac}, _owns{true} {}

        /// @brief Internal send implementation
        /// @param data Pointer to data buffer
        /// @param len Length in bytes
        /// @return Success or translated ESP‑NOW error
        [[nodiscard]] WriteResult processSend(const void *data, usize len) noexcept {
            const auto result = esp_now_send(_mac_address.data(), static_cast<const u8 *>(data), len);

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspError(result));
            }
        }

        KF_IMPL_WRITABLE(Peer, WriteResult);

        WriteResult writeBufferImpl(Slice<const u8> buffer) noexcept {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) { return error(Error::create(Error::TooBigMessage)); }
            return processSend(buffer.data(), buffer.size());
        }

        template<typename T> WriteResult writePacketImpl(T &&packet) noexcept {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return processSend(static_cast<const void *>(&packet), sizeof(T));
        }

        template<typename T> WriteResult writeMixedImpl(T &&header, Slice<const u8> buffer) noexcept {
            const auto mixed_size = sizeof(T) + buffer.size();
            u8 mixed[mixed_size];

            const auto header_data = reinterpret_cast<const u8 *>(&header);
            std::copy(header_data, header_data + sizeof(T), mixed);
            std::copy(buffer.begin(), buffer.end(), mixed + sizeof(T));

            return processSend(static_cast<const void *>(mixed), mixed_size);
        }
    };

private:
    friend struct ::kf::mixin::Singleton<EspNow>;

    /// @brief Private constructor (singleton)
    constexpr EspNow() noexcept : MacAddressed{{}} {}

    /// @brief ESP‑NOW receive callback (static wrapper)
    /// @param raw_mac_address Source MAC address (6 bytes)
    /// @param data Received data buffer
    /// @param size Size of received data
    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) noexcept {
        auto &self = EspNow::instance();

        MacAddress source_mac_address;
        std::copy(raw_mac_address, raw_mac_address + ESP_NOW_ETH_ALEN, source_mac_address.begin());

        const Slice<const u8> buffer{data, static_cast<usize>(size)};

        self.invoke(source_mac_address, buffer);
    }

    /// @brief Translate ESP error code to Error enum
    /// @param result ESP error code
    /// @return Corresponding Error enum value
    [[nodiscard]] static Error translateEspError(esp_err_t result) noexcept {
        switch (result) {
            case ESP_ERR_ESPNOW_INTERNAL: return Error::create(Error::InternalError);
            case ESP_ERR_ESPNOW_NOT_INIT: return Error::create(Error::NotInitialized);
            case ESP_ERR_ESPNOW_ARG: return Error::create(Error::InvalidArg);
            case ESP_ERR_ESPNOW_NO_MEM: return Error::create(Error::NoMemory);
            case ESP_ERR_ESPNOW_NOT_FOUND: return Error::create(Error::PeerNotFound);
            case ESP_ERR_ESPNOW_IF: return Error::create(Error::IncorrectWiFiMode);
            case ESP_ERR_ESPNOW_FULL: return Error::create(Error::PeerListIsFull);
            case ESP_ERR_ESPNOW_EXIST: return Error::create(Error::PeerAlreadyExists);
            default: return Error::create(Error::UnknownError);
        }
    }

    KF_IMPL_INITABLE(EspNow, Result<void, Error>);
    Result<void, Error> initImpl() noexcept {
        (void) esp_read_mac(_mac_address.data(), ESP_MAC_WIFI_STA);

        const auto init_result = esp_now_init();
        if (ESP_OK != init_result) { return error(translateEspError(init_result)); }

        const auto handler_result = esp_now_register_recv_cb(onReceive);
        if (ESP_OK != handler_result) { return error(translateEspError(handler_result)); }

        return ok();
    }

    KF_IMPL_QUITABLE(EspNow);
    void quitImpl() noexcept {
        (void) esp_now_unregister_recv_cb();
        (void) esp_now_deinit();
    }
};

}// namespace kf::network
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nvs.h>
#include <nvs_flash.h>

#include <kf/Option.hpp>
#include <kf/Result.hpp>
#include <kf/Slice.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/Quitable.hpp>
#include <kf/primitives.hpp>

namespace djc::internal {

struct NvsError {
    enum Kind : char {
        InitFailed,
        ReadFailed,
        WriteFailed,
        CommitFailed,
        InvalidLength,
        NotFound,
        NotEnoughSpace,
        Unknown,
    } kind;

    static constexpr auto fromEsp(const esp_err_t e) -> kf::internal::ResultErrorWrapper<NvsError> {
        switch (e) {
            case ESP_ERR_NVS_NOT_FOUND:
                return {NotFound};

            case ESP_ERR_NVS_INVALID_LENGTH:
                return {InvalidLength};

            case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
                return {NotEnoughSpace};

            case ESP_ERR_NVS_INVALID_HANDLE:
            case ESP_ERR_NVS_READ_ONLY:
            case ESP_ERR_NVS_INVALID_NAME:
            default:
                return {Unknown};
        }
    }
};

}// namespace djc::internal

namespace djc::memory {

/// @brief NVS wrapper
struct NVS final :

    kf::mixin::NonCopyable,
    kf::mixin::Initable<NVS, kf::Result<void, internal::NvsError>()>,
    kf::mixin::Quitable<NVS>

{

    using Error = internal::NvsError;

    using ResultType = kf::Result<void, Error>;

    /// @brief Construct NVS storage component
    constexpr explicit NVS(const char *nvs_namespace) noexcept :
        _namespace{nvs_namespace} {}

    [[nodiscard]] ResultType load(kf::Slice<kf::u8> buffer) noexcept {
        auto len = buffer.size();
        return wrap(nvs_get_blob(_handle.unwrap(), blob_key, static_cast<void *>(buffer.data()), &len));
    }

    [[nodiscard]] ResultType dump(kf::Slice<const kf::u8> buffer) noexcept {
        return wrap(nvs_set_blob(_handle.unwrap(), blob_key, buffer.data(), buffer.size()));
    }

    [[nodiscard]] ResultType commit() noexcept {
        return wrap(nvs_commit(_handle.unwrap()));
    }

private:
    static constexpr auto blob_key{"blob"};

    const char *_namespace;
    kf::TrivialOption<nvs_handle_t> _handle{kf::none};

    [[nodiscard]] static ResultType wrap(esp_err_t e) noexcept {
        if (ESP_OK == e) {
            return kf::ok();
        } else {
            return Error::fromEsp(e);
        }
    }

    KF_IMPL_INITABLE(NVS, kf::Result<void, Error>());
    auto initImpl() noexcept -> kf::Result<void, Error> {
        if (_handle.isSome()) {
            // Already initialised
            return kf::ok();
        }

        esp_err_t e;
        nvs_handle_t handle;

        // Ensure NVS flash is initialised (idempotent)
        e = nvs_flash_init();
        if (e == ESP_ERR_NVS_NO_FREE_PAGES or e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated, need to erase and retry
            e = nvs_flash_erase();
            if (e != ESP_OK) {
                return Error::fromEsp(e);
            }
            e = nvs_flash_init();
        }

        if (e != ESP_OK) {
            return Error::fromEsp(e);
        }

        e = nvs_open(_namespace, NVS_READWRITE, &handle);
        if (ESP_OK != e) {
            return Error::fromEsp(e);
        }

        _handle = kf::someTrivial(handle);
        return kf::ok();
    }

    KF_IMPL_QUITABLE(NVS);
    void quitImpl() noexcept {
        if (_handle.isSome()) {
            nvs_close(_handle.unwrap());
            _handle.reset();
        }
    }
};

}// namespace djc::memory
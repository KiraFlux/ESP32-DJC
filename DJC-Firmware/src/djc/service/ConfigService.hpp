// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <utility>

#include <kf/Function.hpp>
#include <kf/Logger.hpp>
#include <kf/Slice.hpp>
#include <kf/math/Timer.hpp>
#include <kf/memory/StaticString.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/primitives.hpp>

#include "djc/math.hpp"
#include "djc/memory/NVS.hpp"
#include "djc/service/Service.hpp"

namespace djc::internal {

using ConfigView = kf::Slice<kf::u8>;

using CallbackedByConfigView = kf::mixin::Callbacked<ConfigView>;

struct ConfigServiceOnLoadCallbacked : private CallbackedByConfigView {

    /// @brief Set config service behavior on load
    template<typename F> void onLoad(F &&f) noexcept {
        this->callback(std::forward<F>(f));
    }

protected:
    void invokeOnLoad(ConfigView view) noexcept {
        this->invoke(view);
    }
};

struct ConfigServiceResettingStrategy : private CallbackedByConfigView {

    /// @brief Set config service resetting strategy
    template<typename F> void resettingStrategy(F &&f) noexcept {
        this->callback(std::forward<F>(f));
    }

protected:
    void invokeResetStrategy(ConfigView view) noexcept {
        this->invoke(view);
    }
};

}// namespace djc::internal

namespace djc::service {

/// @brief Config service with delayed NVS operations
/// @note Requests are batched and executed on a 5-second timer from the main loop.
struct ConfigService :

    Service<ConfigService>,
    internal::ConfigServiceOnLoadCallbacked,
    internal::ConfigServiceResettingStrategy

{
    explicit constexpr ConfigService(const char *nvs_namespace, const kf::math::Timer::Config &sync_timer_config, kf::Slice<kf::u8> config_view) noexcept :
        _nvs{nvs_namespace}, _sync_timer{sync_timer_config}, _config_view{config_view} {}

    /// @brief Requests an deferred load of the config from NVS
    void requestLoad() noexcept {
        _load_requested = true;
        logger.debug("Load requested");
    }

    /// @brief Requests an deferred reset of the config to defaults
    void requestReset() noexcept {
        _reset_requested = true;
        logger.debug("Reset requested");
    }

    /// @brief Calculate CRC32 for config view
    [[nodiscard]] kf::u32 crc() const noexcept {
        return djc::math::crc32(_config_view);
    }

    /// @brief Force sync now
    void sync() noexcept {
        using LogString = kf::memory::StaticString<64>;

        // init is idempotent
        if (_nvs.init().isError()) {
            logger.error(LogString::formatted("NVS(%s) init failed", _nvs.name()));
        }

        if (_load_requested) {
            _load_requested = false;

            logger.info(LogString::formatted("Loading config '%s' from NVS...", _nvs.name()));

            if (_nvs.getBlob(blob_key, _config_view).isOk()) {
                _stored_crc = crc();
                logger.info(LogString::formatted("Config '%s' loaded from NVS (CRC: %u)", _nvs.name(), _stored_crc).view());

                this->invokeOnLoad(_config_view);

            } else {
                logger.error(LogString::formatted("Config '%s' load failed", _nvs.name()));
                requestReset();
            }
        }

        if (_reset_requested) {
            _reset_requested = false;

            this->invokeResetStrategy(_config_view);
            logger.info(LogString::formatted("Config reset '%s' to defaults", _nvs.name()));
        }

        if (const auto current_crc = crc(); current_crc != _stored_crc) {
            logger.info(LogString::formatted("Config '%s' changed, saving (CRC: %u -> %u)...", _nvs.name(), _stored_crc, current_crc).view());

            if (_nvs.setBlob(blob_key, _config_view).isOk() and _nvs.commit().isOk()) {
                _stored_crc = current_crc;
                logger.info(LogString::formatted("Config '%s' saved, CRC updated", _nvs.name()));
            } else {
                logger.error(LogString::formatted("Config '%s' save failed", _nvs.name()));
            }
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("ConfigService")};

    static constexpr auto blob_key{"blob"};

    memory::NVS _nvs;
    kf::Slice<kf::u8> _config_view;
    kf::math::Timer _sync_timer;
    kf::u32 _stored_crc{};
    bool _load_requested{false}, _reset_requested{false};

    KF_IMPL_TIMED_POLLABLE(ConfigService);
    void pollImpl(kf::math::Milliseconds now) noexcept {
        if (_sync_timer.expired(now)) {
            _sync_timer.start(now);
            sync();
        }
    }
};

}// namespace djc::service
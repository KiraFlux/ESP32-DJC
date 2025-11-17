#pragma once

#include <WiFi.h>
#include <functional>
#include <kf/Logger.hpp>
#include <kf/Result.hpp>
#include <kf/String.hpp>
#include <kf/aliases.hpp>
#include <kf/espnow.hpp>

namespace djc {

struct EspnowNode {
    kf::espnow::Mac target;

    std::function<void(kf::slice<const void>)> on_receive{nullptr};

    [[nodiscard]] bool init() const {
        if (not WiFiClass::mode(WIFI_MODE_STA)) {
            kf_Logger_error("WIFI mode set fail");
            return false;
        }

        if (const auto result = kf::espnow::Protocol::init(); not result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(result.error().value()));
            return false;
        }

        if (const auto result = kf::espnow::Peer::add(target); not result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(result.error().value()));
            return false;
        }

        auto handler = [this](const kf::espnow::Mac &mac, kf::slice<const void> data) {
            if (on_receive == nullptr) { return; }
            // if (mac != target) { return; }
            on_receive(data);
        };

        if (const auto result = kf::espnow::Protocol::instance().setReceiveHandler(handler); not result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(result.error().value()));
            return false;
        }

        kf_Logger_info("OK");
        return true;
    }

    template<typename T> [[nodiscard]] inline kf::Result<void, kf::espnow::Error> send(const T &value) const {
        return kf::espnow::Protocol::send(target, value);
    }

    [[nodiscard]] inline kf::Result<void, kf::espnow::Error> send(kf::slice<const void> data) const {
        return kf::espnow::Protocol::send(target, data);
    }
};

}// namespace djc

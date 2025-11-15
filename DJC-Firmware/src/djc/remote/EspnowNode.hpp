#pragma once

#include <WiFi.h>
#include <functional>
#include <kf/Logger.hpp>
#include <kf/espnow.hpp>

namespace djc {

struct EspnowNode {
    kf::espnow::Mac target;

    std::function<void(const void *data, const kf::u8 len)> on_receive{nullptr};

    [[nodiscard]] bool init() const {
        if (not WiFiClass::mode(WIFI_MODE_STA)) {
            kf_Logger_error("WIFI mode set fail");
            return false;
        }

        if (const auto result = kf::espnow::Protocol::init(); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        if (const auto result = kf::espnow::Peer::add(target); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        auto handler = [this](const kf::espnow::Mac &mac, const void *data, kf::u8 len) {
            if (on_receive == nullptr) { return; }
            // if (mac != target) { return; }
            on_receive(data, len);
        };

        if (const auto result = kf::espnow::Protocol::instance().setReceiveHandler(handler); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        kf_Logger_info("OK");
        return true;
    }

    template<typename T> [[nodiscard]] inline rs::Result<void, kf::espnow::Error> send(const T &value) const {
        return kf::espnow::Protocol::send(target, value);
    }

    [[nodiscard]] inline rs::Result<void, kf::espnow::Error> send(const void *data, rs::u8 len) const {
        return kf::espnow::Protocol::send(target, data, len);
    }
};

}// namespace djc

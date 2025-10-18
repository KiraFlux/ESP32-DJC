#pragma once

#include <WiFi.h>
#include <espnow/Protocol.hpp>
#include <kf/Logger.hpp>

namespace djc {

struct EspnowNode {
    espnow::Mac target;

    [[nodiscard]] bool init() const {
        if (not WiFiClass::mode(WIFI_MODE_STA)) {
            kf_Logger_error("WIFI mode set fail");
            return false;
        }

        if (const auto result = espnow::Protocol::init(); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        if (const auto result = espnow::Peer::add(target); result.fail()) {
            kf_Logger_error(rs::toString(result.error));
            return false;
        }

        kf_Logger_info("OK");
        return true;
    }

    template<typename T> inline rs::Result<void, espnow::Protocol::SendError> send(const T &value) {
        return espnow::Protocol::send(target, value);
    }

    inline rs::Result<void, espnow::Protocol::SendError> send(const void *data, rs::u8 len) {
        return espnow::Protocol::send(target, data, len);
    }
};

}// namespace djc

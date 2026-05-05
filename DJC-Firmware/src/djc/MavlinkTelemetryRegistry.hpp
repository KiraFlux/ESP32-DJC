// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <MAVLink.h>

#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>

namespace djc {

/// @brief Centralised storage for received MAVLink telemetry messages
/// @note Holds a set of typed entries, one for each supported MAVLink message type.
struct MavlinkTelemetryRegistry final : kf::mixin::NonCopyable {

    /// @brief Typed telemetry entry with own decoder and freshness tracking
    template<typename T> struct Entry : kf::mixin::NonCopyable {

        using ValueType = T;
        using MessageDecoder = void (*)(const mavlink_message_t *message, ValueType *);

        explicit constexpr Entry(MessageDecoder message_decoder) noexcept :
            _message_decoder{message_decoder} {}

        [[nodiscard]] const ValueType &value() const noexcept { return _value; }

        /// @brief Check if the entry was updated after the given timestamp
        [[nodiscard]] bool updatedSince(kf::math::Milliseconds since) const noexcept {
            return _last_update_time > since;
        }

        /// @brief Decode a MAVLink message into this entry and record the update time
        void update(kf::math::Milliseconds now, const mavlink_message_t &message) noexcept {
            if (nullptr != _message_decoder) {
                _message_decoder(&message, &_value);
                _last_update_time = now;
            }
        }

    private:
        const MessageDecoder _message_decoder;
        ValueType _value{};
        kf::math::Milliseconds _last_update_time{0u};
    };

    Entry<mavlink_heartbeat_t> heartbeat{mavlink_msg_heartbeat_decode};
    Entry<mavlink_attitude_quaternion_t> attitude_quaternion{mavlink_msg_attitude_quaternion_decode};
    Entry<mavlink_serial_control_t> serial_control{mavlink_msg_serial_control_decode};
    Entry<mavlink_scaled_imu_t> scaled_imu{mavlink_msg_scaled_imu_decode};

    /// @brief Route an incoming MAVLink message to the correct entry based on its ID
    /// @note Only known message types are dispatched; unknown IDs are silently ignored.
    void update(kf::math::Milliseconds now, const mavlink_message_t &message) noexcept {
        switch (message.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                heartbeat.update(now, message);
                return;

            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION:
                attitude_quaternion.update(now, message);
                return;

            case MAVLINK_MSG_ID_SERIAL_CONTROL:
                serial_control.update(now, message);
                return;

            case MAVLINK_MSG_ID_SCALED_IMU:
                scaled_imu.update(now, message);
                return;
        }
    }
};

}// namespace djc
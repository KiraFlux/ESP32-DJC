// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <MAVLink.h>

#include <kf/math/units.hpp>
#include <kf/mixin/NonCopyable.hpp>

namespace djc {

struct MavlinkTelemetryRegistry final : kf::mixin::NonCopyable {

    kf::math::Milliseconds last_update_time;
    mavlink_heartbeat_t heartbeat;
    mavlink_attitude_quaternion_t attitude_quaternion;
    mavlink_serial_control_t serial_control;
    mavlink_scaled_imu_t scaled_imu;

    void update(kf::math::Milliseconds now, const mavlink_message_t &message) noexcept {
        constexpr auto serial_control_max_len{sizeof(mavlink_serial_control_t::data)};

        last_update_time = now;

        switch (message.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                mavlink_msg_heartbeat_decode(&message, &heartbeat);
                return;

            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION:
                mavlink_msg_attitude_quaternion_decode(&message, &attitude_quaternion);
                return;

            case MAVLINK_MSG_ID_SERIAL_CONTROL:
                mavlink_msg_serial_control_decode(&message, &serial_control);
                serial_control.data[serial_control_max_len - 1] = '\0';
                return;

            case MAVLINK_MSG_ID_SCALED_IMU:
                mavlink_msg_scaled_imu_decode(&message, &scaled_imu);
                return;
        }
    }

    [[nodiscard]] bool updated(kf::math::Milliseconds last_seen_time) const noexcept {
        return last_seen_time < last_update_time;
    }
};

}// namespace djc
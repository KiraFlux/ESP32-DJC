// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <MAVLink.h>

#include <kf/aliases.hpp>
#include <kf/math/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Slice.hpp>
#include <kf/mixin/Callbacked.hpp>
#include <kf/mixin/Configurable.hpp>

#include "djc/ManualInput.hpp"
#include "djc/protocol/Protocol.hpp"
#include "djc/transport/TransportLink.hpp"

namespace djc::protocol {

namespace internal {

/// @brief Configuration for the MAVLink protocol.
struct MavlinkProtocolConfig {

    using IdType = kf::u8;

    kf::math::Milliseconds heartbeat_period;///< Period between HEARTBEAT messages (ms).
    IdType system_id_self;                  ///< MAVLink system ID of this controller.
    IdType system_id_target;                ///< MAVLink system ID of the target drone (0 = broadcast).
    IdType component_id_heartbeat;
    IdType component_id_manual_control;

    static constexpr MavlinkProtocolConfig defaults() noexcept {
        return MavlinkProtocolConfig{
            .heartbeat_period = 2'000,// ms
            .system_id_self = 0x7f,
            .system_id_target = 0x01,
            .component_id_heartbeat = MAV_COMP_ID_USER1,
            .component_id_manual_control = MAV_COMP_ID_USER2,
        };
    }
};

}// namespace internal

/// @brief MAVLink protocol - sends MANUAL_CONTROL and HEARTBEAT packets, invokes callback on received MAVLink messages
/// @note
/// A manual control message is sent on every `poll()`, carrying the current stick values.
/// A heartbeat is sent immediately on activation and then at the configured period.
/// Incoming data is parsed as MAVLink and forwarded via callback.
struct MavlinkProtocol :

    Protocol,
    kf::mixin::Callbacked<const mavlink_message_t &>,
    kf::mixin::Configurable<internal::MavlinkProtocolConfig>

{
    using Config = internal::MavlinkProtocolConfig;

    using kf::mixin::Configurable<Config>::Configurable;

    /// @brief Serialise and send a MAVLink message through the given transport.
    /// @param transport_link Transport to use for sending.
    /// @param message The message to send.
    /// @return true if the transport reported success, false otherwise.
    /// @note
    /// The return value reflects the transport-level result.
    [[nodiscard]] bool sendMessage(transport::TransportLink &transport_link, const mavlink_message_t &message) const noexcept {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        return transport_link.send({buffer, len});
    }

    // impl dynamic

    void poll(kf::math::Milliseconds now, const ManualInput &input, transport::TransportLink &transport_link) noexcept override {
        if (_heartbear_timer.expired(now) or _need_to_reset_heartbeat_timer) {
            _need_to_reset_heartbeat_timer = false;
            _heartbear_timer.start(now);

            sendHeartbeat(transport_link);
        }

        sendManualControl(transport_link, input);
    }

    void receive(kf::memory::Slice<const kf::u8> buffer) noexcept override {
        mavlink_message_t message;
        mavlink_status_t status;

        for (auto b: buffer) {
            if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status) != 0) {
                this->invoke(message);
            }
        }
    }

private:
    kf::math::Timer _heartbear_timer{this->config().heartbeat_period};
    bool _need_to_reset_heartbeat_timer{true};

    void sendHeartbeat(transport::TransportLink &transport_link) const noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_heartbeat_pack(
            this->config().system_id_self,
            this->config().component_id_heartbeat,
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0// Base mode, Custom mode, system status
        );

        (void) sendMessage(transport_link, message);
    }

    void sendManualControl(transport::TransportLink &transport_link, const ManualInput &input) const noexcept {
        mavlink_message_t message;
        (void) mavlink_msg_manual_control_pack(
            this->config().system_id_self,
            this->config().component_id_manual_control,
            &message,
            this->config().system_id_target,
            input.right_y,// x: pitch (right Y)
            input.right_x,// y: roll (right X)
            input.left_y, // z: thrust (left Y)
            input.left_x, // r: yaw (left X)
            // Buttons (unused)
            0, 0,           // buttons
            0,              // extensions
            0, 0,           // roll/pitch only axes
            0, 0, 0, 0, 0, 0// aux0..6
        );

        (void) sendMessage(transport_link, message);
    }
};

}// namespace djc::protocol
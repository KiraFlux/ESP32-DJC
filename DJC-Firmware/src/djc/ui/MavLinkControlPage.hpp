#pragma once

#include <Arduino.h>
#include <MAVLink.h>
#include <cstring>

#include <kf/Logger.hpp>
#include <kf/math/time/Timer.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/Device.hpp"
#include "djc/UI.hpp"
#include "djc/ui/MainPage.hpp"


namespace djc {

/// @brief MAVLink protocol control page for drone/vehicle control
struct MavLinkControlPage : UI::Page {
private:
    static constexpr auto logger{kf::Logger::create("MavLink")};

    kf::Timer test_timer{static_cast<kf::Milliseconds>(100)};
    kf::Timer heartbeat_timer{static_cast<kf::Milliseconds>(2000)};
    kf::ArrayString<256> log_buffer{"\xB1""Wonderful emptiness..."};
    kf::StringView log_view{log_buffer.view()};

    UI::Display <kf::StringView> log_display{*this, log_view};

public:
    explicit MavLinkControlPage() :
        Page{"MAV Link Control"} {
        link(MainPage::instance());
    }

    void onEntry() noexcept override {
        kf::EspNow::instance().setUnknownReceiveHandler(
            [this](const kf::EspNow::Mac &, kf::Slice<const kf::u8> buffer) {
                mavlink_message_t message;
                mavlink_status_t status;

                for (auto b: buffer) {
                    if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status)) {
                        onMavLinkMessage(&message);
                    }
                }
            }
        );
    }

    void onUpdate(kf::Milliseconds now) noexcept override {
        sendManualControl();

        if (heartbeat_timer.ready(now)) {
            sendHeartBeat();
        }
    }

private:
    void onMavLinkMessage(mavlink_message_t *message) {
        logger.debug(kf::ArrayString<32>::formatted("MAVLink message ID: %d", message->msgid).view());

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(log_buffer.data(), serial_control.data, serial_control.count);
                log_buffer[kf::min(kf::usize(serial_control.count), log_buffer.size() - 1)] = '\0';
                break;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) log_buffer.format(
                    "Accel: %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f
                );
                break;
            }

            default:
                // Unhandled message type
                break;
        }
    }

    static void sendHeartBeat() {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127, // System ID
            MAV_COMP_ID_OSD, // Component ID
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            0, 0, 0 // Custom mode, system status, MAVLink version
        );

        sendMavlinkToEspnow(message);
    }

    void sendManualControl() {
        constexpr auto scale = 1000;

        const auto &controller_values = Device::instance().controllerValues();
        const auto left_x = controller_values.left_x * scale;
        const auto left_y = controller_values.left_y * scale;
        const auto right_x = controller_values.right_x * scale;
        const auto right_y = controller_values.right_y * scale;

        if (test_timer.ready(millis())) {
            logger.debug(kf::ArrayString<64>::formatted(
                "L: (%.3f, %.3f) R: (%.3f, %.3f)",
                left_x, left_y, right_x, right_y
            ).view());
        }

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x: pitch (right Y)
            static_cast<kf::i16>(right_y),
            // y: roll (right X)
            static_cast<kf::i16>(right_x),
            // z: thrust (left Y)
            static_cast<kf::i16>(left_y),
            // r: yaw (left X)
            static_cast<kf::i16>(left_x),
            // Buttons (unused)
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        );

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);

        auto &peer_opt = Device::instance().espnowPeer();
        if (peer_opt.hasValue()) {
            (void) peer_opt.value().sendBuffer(kf::Slice<const kf::u8>{buffer, len});
        }
    }
};

} // namespace djc
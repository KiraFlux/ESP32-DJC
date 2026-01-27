#pragma once

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

struct MavLinkControlPage : UI::Page {

private:
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
        kf::EspNow::instance().setUnknownReceiveHandler([this](const kf::EspNow::Mac &, kf::Slice<const kf::u8> buffer) {
            mavlink_message_t message;
            mavlink_status_t status;

            for (auto b: buffer) {
                if (mavlink_parse_char(MAVLINK_COMM_0, b, &message, &status)) {
                    onMavLinkMessage(&message);
                }
            }
        });
    }

    void onUpdate(kf::Milliseconds now) noexcept override {
        sendManualControl();

        if (heartbeat_timer.ready(now)) {
            sendHeartBeat();
        }
    }

private:
    void onMavLinkMessage(mavlink_message_t *message) {
        kf_Logger_debug("%d", message->msgid);

        switch (message->msgid) {
            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;

                mavlink_msg_serial_control_decode(message, &serial_control);

                std::memcpy(log_buffer.data(), serial_control.data, serial_control.count);
                log_buffer[kf::min(kf::usize(serial_control.count), log_buffer.size() - 1)] = '\0';
            }
                return;

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;

                mavlink_msg_scaled_imu_decode(message, &imu);

                (void) log_buffer.format(
                    "A %.2f %.2f %.2f",
                    kf::f32(imu.xacc) * 0.001f,
                    kf::f32(imu.yacc) * 0.001f,
                    kf::f32(imu.zacc) * 0.001f);
            }

            default://
                return;
        }
    }

    //    static void sendSerialControl() {
    //        mavlink_message_t message;
    //        // mavlink_msg_serial_control_encode(
    //        //     127,
    //
    //        // );
    //        sendMavlinkToEspnow(message);
    //    }

    static void sendHeartBeat() {
        mavlink_message_t message;
        mavlink_msg_heartbeat_pack(
            127,
            MAV_COMP_ID_OSD,
            &message,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            //
            0, 0, 0);

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
            kf_Logger_debug(
                "L: (%2.3f\t%2.3f)\tR: (%2.3f\t%2.3f)",
                left_x, left_y, right_x, right_y
            );
        }

        mavlink_message_t message;
        mavlink_msg_manual_control_pack(
            127, MAV_COMP_ID_PARACHUTE, &message, 1,
            // x : pitch
            static_cast<kf::i16>(right_y),
            // y : roll
            static_cast<kf::i16>(right_x),
            // z : thrust
            static_cast<kf::i16>(left_y),
            // r : yaw
            static_cast<kf::i16>(left_x),
            //
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        sendMavlinkToEspnow(message);
    }

    static void sendMavlinkToEspnow(mavlink_message_t &message) {
        kf::u8 buffer[MAVLINK_MAX_PACKET_LEN];
        const auto len = mavlink_msg_to_send_buffer(buffer, &message);
        auto &o = Device::instance().espnowPeer();
        if (o.hasValue()) {
            (void) o.value().sendBuffer(kf::Slice<const kf::u8>{buffer, len});
        }
    }
};

}// namespace djc
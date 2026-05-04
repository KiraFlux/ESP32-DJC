// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <MAVLink.h>

#include <kf/Logger.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/protocol/MavlinkProtocol.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief MAVLink telemetry page
struct MavlinkTelemetryPage : UI::Page {
    explicit MavlinkTelemetryPage(UI::Page &root, protocol::ProtocolRegistry &protocol_registry, protocol::ProtocolLink &protocol_link) noexcept :
        Page{"Mavlink: Telemetry"}, _protocol_registry{protocol_registry}, _protocol_link{protocol_link},

        _layout{{
            &root.link(),
            &_imu_display,
            &_attitude_display,
        }} {
        widgets({_layout.data(), _layout.size()});
    }

    void onEntry() noexcept override {
        _protocol_registry.mavlink().callback([this](const mavlink_message_t &message) {
            _need_update |= onMavlinkMessage(message);
        });
        _protocol_link.protocol(_protocol_registry.mavlink());
    }

    void onExit() noexcept override {
        _protocol_registry.mavlink().callback(protocol::MavlinkProtocol::CallbackType{});
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        if (_need_update) {
            _need_update = false;
            UI::instance().addEvent(UI::Event::update());
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("MavlinkTelemetryPage")};

    protocol::ProtocolRegistry &_protocol_registry;
    protocol::ProtocolLink &_protocol_link;
    bool _need_update{false};

    // widgets

    kf::memory::ArrayString<64> _attitude_buffer{"..."};
    kf::memory::ArrayString<64> _imu_display_buffer{"..."};

    UI::Display<kf::memory::StringView> _attitude_display{_attitude_buffer.view()};
    UI::Display<kf::memory::StringView> _imu_display{_imu_display_buffer.view()};

    kf::memory::Array<UI::Widget *, 3> _layout;

    [[nodiscard]] bool onMavlinkMessage(const mavlink_message_t &message) noexcept {
        switch (message.msgid) {
            case MAVLINK_MSG_ID_ATTITUDE_QUATERNION: {
                mavlink_attitude_quaternion_t attitude_quaternion;
                mavlink_msg_attitude_quaternion_decode(&message, &attitude_quaternion);

                (void) _attitude_buffer.format(
                    "AtQ %+.2f %+.2f %+.2f %+.2f",
                    float(attitude_quaternion.q1),
                    float(attitude_quaternion.q2),
                    float(attitude_quaternion.q3),
                    float(attitude_quaternion.q4));
                _attitude_display.value(_attitude_buffer.view());

                return true;
            }

            case MAVLINK_MSG_ID_SERIAL_CONTROL: {
                mavlink_serial_control_t serial_control;
                mavlink_msg_serial_control_decode(&message, &serial_control);

                constexpr auto len{sizeof(serial_control.data)};
                serial_control.data[len - 1] = '\0';

                logger.info({reinterpret_cast<const char *>(serial_control.data), static_cast<kf::usize>(serial_control.count)});

                return false;
            }

            case MAVLINK_MSG_ID_SCALED_IMU: {
                mavlink_scaled_imu_t imu;
                mavlink_msg_scaled_imu_decode(&message, &imu);

                (void) _imu_display_buffer.format(
                    "Acc %+.3f %+.3f %+.3f",
                    float(imu.xacc * 0.001f),
                    float(imu.yacc * 0.001f),
                    float(imu.zacc * 0.001f));
                _imu_display.value(_imu_display_buffer.view());

                return true;
            }

            default:
                // Unhandled message type
                return false;
        }
    }
};

}// namespace djc::ui::pages
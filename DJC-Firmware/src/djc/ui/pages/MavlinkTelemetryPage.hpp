// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <MAVLink.h>

#include <kf/Logger.hpp>
#include <kf/math/units.hpp>
#include <kf/memory/Array.hpp>
#include <kf/memory/ArrayString.hpp>
#include <kf/memory/StringView.hpp>

#include "djc/MavlinkTelemetryRegistry.hpp"
#include "djc/protocol/MavlinkProtocol.hpp"
#include "djc/protocol/ProtocolLink.hpp"
#include "djc/protocol/ProtocolRegistry.hpp"
#include "djc/ui/UI.hpp"

namespace djc::ui::pages {

/// @brief MAVLink telemetry page
struct MavlinkTelemetryPage : UI::Page {
    explicit MavlinkTelemetryPage(
        UI::Page &root,
        protocol::ProtocolRegistry &protocol_registry,
        protocol::ProtocolLink &protocol_link,
        MavlinkTelemetryRegistry &mavlink_telemetry_registry) noexcept :
        Page{"Mavlink: Telemetry"},
        _protocol_registry{protocol_registry},
        _protocol_link{protocol_link},
        _mavlink_telemetry_registry{mavlink_telemetry_registry},

        _layout{{
            &root.link(),
            &_imu_display,
            &_attitude_display,
        }} {
        widgets({_layout.data(), _layout.size()});
    }

    void onEntry() noexcept override {
        _protocol_link.protocol(_protocol_registry.mavlink());
        _last_imu = _last_attitude = _last_serial_control = 0;
    }

    void onUpdate(kf::math::Milliseconds now) noexcept override {
        bool need_update{false};

        if (_mavlink_telemetry_registry.scaled_imu.updatedSince(_last_imu)) {
            _last_imu = now;
            const auto &imu = _mavlink_telemetry_registry.scaled_imu.value();

            (void) _imu_buffer.format(
                "Acc %+.3f %+.3f %+.3f",
                float(imu.xacc * 0.001f),
                float(imu.yacc * 0.001f),
                float(imu.zacc * 0.001f));
            _imu_display.value(_imu_buffer.view());

            need_update = true;
        }

        if (_mavlink_telemetry_registry.attitude_quaternion.updatedSince(_last_attitude)) {
            _last_attitude = now;
            const auto &attitude = _mavlink_telemetry_registry.attitude_quaternion.value();

            (void) _attitude_buffer.format(
                "Q %+.2f %+.2f %+.2f %+.2f",
                attitude.q1,
                attitude.q2,
                attitude.q3,
                attitude.q4);
            _attitude_display.value(_attitude_buffer.view());

            need_update = true;
        }

        if (_mavlink_telemetry_registry.serial_control.updatedSince(_last_serial_control)) {
            _last_serial_control = now;
            const auto &s = _mavlink_telemetry_registry.serial_control.value();

            logger.debug({reinterpret_cast<const char *>(s.data), s.count});// temp.
        }

        if (need_update) {
            UI::instance().addEvent(UI::Event::update());
        }
    }

private:
    static constexpr auto logger{kf::Logger::create("MavlinkTelemetryPage")};

    protocol::ProtocolRegistry &_protocol_registry;
    protocol::ProtocolLink &_protocol_link;
    MavlinkTelemetryRegistry &_mavlink_telemetry_registry;
    kf::math::Milliseconds _last_imu{}, _last_attitude{}, _last_serial_control{};

    // widgets
    kf::memory::ArrayString<64> _attitude_buffer{"..."}, _imu_buffer{"..."};

    UI::Display<kf::memory::StringView> _attitude_display{_attitude_buffer.view()}, _imu_display{_imu_buffer.view()};

    kf::memory::Array<UI::Widget *, 3> _layout;
};

}// namespace djc::ui::pages
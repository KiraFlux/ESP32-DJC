// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/bus/spi/ArduinoSPI.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/drivers/sensors/Joystick.hpp>
#include <kf/drivers/sensors/NormalizedAdcInput.hpp>
#include <kf/gpio/arduino.hpp>
#include <kf/network/EspNow.hpp>

#include "djc/input/Button.hpp"

namespace djc {

using namespace kf::gpio::arduino;

using Button = djc::input::Button<DigitalInput>;

using AxisInput = kf::drivers::sensors::NormalizedAdcInput<AdcInput>;
using Joystick = kf::drivers::sensors::Joystick<AxisInput>;

using Bus = kf::bus::spi::ArduinoSPI;
using DisplayDriver = kf::drivers::display::ST7735<Bus::Node, DigitalOutput>;

using EspNow = kf::network::EspNow;

}// namespace djc
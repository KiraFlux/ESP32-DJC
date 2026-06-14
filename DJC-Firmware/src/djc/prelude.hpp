// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/bus/spi/ArduinoSPI.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/drivers/sensors/Joystick.hpp>
#include <kf/drivers/sensors/NormalizedAdcInput.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>
#include <kf/input/LogicalLevelListener.hpp>

namespace djc {

using GPIO = kf::gpio::ArduinoGPIO;

using ButtonListener = kf::input::LogicalLevelListener<GPIO::DigitalInput>;

using AxisInput = kf::drivers::sensors::NormalizedAdcInput<GPIO::AdcInput>;

using Joystick = kf::drivers::sensors::Joystick<AxisInput>;

using SpiBus = kf::bus::spi::ArduinoSPI;

using DisplayDriver = kf::drivers::display::ST7735<SpiBus::Node, GPIO::DigitalOutput>;

}// namespace djc
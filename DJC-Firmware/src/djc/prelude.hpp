// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/bus/spi/ArduinoSPI.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/drivers/sensors/Joystick.hpp>
#include <kf/drivers/sensors/NormalizedAdcInput.hpp>
#include <kf/gpio/arduino.hpp>
#include <kf/input/Button.hpp>
#include <kf/input/JoystickListener.hpp>
#include <kf/network/EspNow.hpp>

namespace djc {

using namespace kf::gpio::arduino;

using Button = kf::input::Button<DigitalInput>;

using AxisInput = kf::drivers::sensors::NormalizedAdcInput<AdcInput>;
using Joystick = kf::drivers::sensors::Joystick<AxisInput>;
using JoystickListener = kf::input::JoystickListener<Joystick>;

using Bus = kf::bus::spi::ArduinoSPI;
using DisplayDriver = kf::drivers::display::ST7735<Bus::Node, DigitalOutput>;

using kf::network::EspNow;

}// namespace djc
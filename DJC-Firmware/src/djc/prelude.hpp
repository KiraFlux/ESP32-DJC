// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Arduino.h>

#include <kf/gpio/ArduinoGPIO.hpp>

#include <kf/bus/iic/ArduinoIIC.hpp>
#include <kf/bus/spi/ArduinoSPI.hpp>

#include <kf/drivers/display/ST7735.hpp>
#include <kf/drivers/sensors/Joystick.hpp>
#include <kf/drivers/sensors/NormalizedAdcInput.hpp>

#include <kf/input/LogicalLevelListener.hpp>

namespace djc {

constexpr gpio_num_t

    // buttons

    gpio_button_left{GPIO_NUM_14},
    gpio_button_right{GPIO_NUM_4},

    // joystick axis

    gpio_joystick_left_x{GPIO_NUM_32},
    gpio_joystick_left_y{GPIO_NUM_33},
    gpio_joystick_right_x{GPIO_NUM_34},
    gpio_joystick_right_y{GPIO_NUM_35},

    // i2c

    gpio_i2c_sda{GPIO_NUM_21},
    gpio_i2c_scl{GPIO_NUM_22},

    // spi

    gpio_spi_mosi{GPIO_NUM_23},
    gpio_spi_miso{GPIO_NUM_19},
    gpio_spi_sck{GPIO_NUM_18},

    // display

    gpio_display_st7735_spi_cs{GPIO_NUM_5},
    gpio_display_st7735_data_command{GPIO_NUM_16},
    gpio_display_st7735_reset{GPIO_NUM_17}

;

using GPIO = kf::gpio::ArduinoGPIO;

using ButtonListener = kf::input::LogicalLevelListener<GPIO::DigitalInput>;

using AxisInput = kf::drivers::sensors::NormalizedAdcInput<GPIO::AdcInput>;

using Joystick = kf::drivers::sensors::Joystick<AxisInput>;

using IicBus = kf::bus::iic::ArduinoIIC;

using SpiBus = kf::bus::spi::ArduinoSPI;

using DisplayDriver = kf::drivers::display::ST7735<SpiBus::Node, GPIO::DigitalOutput>;

}// namespace djc
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Arduino.h>// delay, gpio_num_t

#include <kf/Logger.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/primitives.hpp>

#include <kf/gpio/ArduinoGPIO.hpp>
#include <kf/input/LogicalLevelListener.hpp>// TODO: move to InputHandler

#include <kf/bus/iic/ArduinoIIC.hpp>
#include <kf/bus/spi/ArduinoSPI.hpp>

#include <kf/drivers/display/SSD1306.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/drivers/sensors/Joystick.hpp>
#include <kf/drivers/sensors/NormalizedAdcInput.hpp>

namespace djc {

/// @brief ESP32-DJC Hardware Periphery
struct Periphery final :

    kf::mixin::NonCopyable,
    kf::mixin::Initable<Periphery, void()>

{

    using GPIO = kf::gpio::ArduinoGPIO;

    using ButtonListener = kf::input::LogicalLevelListener<GPIO::DigitalInput>;

    using AxisInput = kf::drivers::sensors::NormalizedAdcInput<GPIO::AdcInput>;

    using Joystick = kf::drivers::sensors::Joystick<AxisInput>;

    using IicBus = kf::bus::iic::ArduinoIIC;

    using SSD1306 = kf::drivers::display::SSD1306<IicBus::Node>;

    using SpiBus = kf::bus::spi::ArduinoSPI;

    using ST7735 = kf::drivers::display::ST7735<SpiBus::Node, GPIO::DigitalOutput>;

#if defined(DJC_DISPLAY_DRIVER_ST7735)

    using DisplayDriver = ST7735;

#elif defined(DJC_DISPLAY_DRIVER_SSD1306)

    using DisplayDriver = SSD1306;

#else

#error DJC_DISPLAY_DRIVER_* Must be defined!

#endif

    static constexpr gpio_num_t

        // inputs

        gpio_button_left{GPIO_NUM_26},
        gpio_button_right{GPIO_NUM_25},

        gpio_joystick_left_x{GPIO_NUM_32},
        gpio_joystick_left_y{GPIO_NUM_33},
        gpio_joystick_right_x{GPIO_NUM_34},
        gpio_joystick_right_y{GPIO_NUM_35},

        gpio_battery_level{GPIO_NUM_39},

        // bus

        gpio_i2c_sda{GPIO_NUM_21},
        gpio_i2c_scl{GPIO_NUM_22},

        gpio_spi_mosi{GPIO_NUM_23},
        gpio_spi_miso{GPIO_NUM_19},
        gpio_spi_sck{GPIO_NUM_18},

        // display

        gpio_display_st7735_spi_cs{GPIO_NUM_5},
        gpio_display_st7735_data_command{GPIO_NUM_16},
        gpio_display_st7735_reset{GPIO_NUM_17}

    ;

    struct Config {

        // Input

        ButtonListener::Config button;

        AxisInput::FilterImpl::Config axis_filter;

        Joystick::Config left_joystick, right_joystick;

        // I2C

        IicBus::Config iic_bus;

        IicBus::Node::Config ssd1306_iic_node;

        // SPI

        SpiBus::Config spi_bus;

        SpiBus::Node::Config st7735_spi_node;

        ST7735::Config st7735;

        // Other

        kf::u16 joystick_axes_tune_samples;

        bool joystick_axes_tuned;

        static constexpr auto defaults() noexcept {
            return Config{
                .button = {
                    .debounce = 50,// ms
                },
                .axis_filter = {
                    .factor = 0.5f,
                },
                .left_joystick = {
                    .x = axisDefaults(true),
                    .y = axisDefaults(false),
                },
                .right_joystick = {
                    .x = axisDefaults(false),
                    .y = axisDefaults(true),
                },
                .iic_bus = IicBus::Config::create(/* clock: */ 400'000, /* timeout: (=default) */ 0, /* buffer_size: */ 0 /* , gpio_i2c_sda, gpio_i2c_scl */),
                .ssd1306_iic_node = {
                    .address = SSD1306::default_address,
                },
                .spi_bus = SpiBus::Config::create(gpio_spi_mosi, gpio_spi_miso, gpio_spi_sck),
                .st7735_spi_node = SpiBus::Node::Config::create(gpio_display_st7735_spi_cs, /* clock: */ 27'000'000),
                .st7735 = {
                    .init_orientation = kf::drivers::display::Orientation::ClockWise,
                },
                .joystick_axes_tune_samples = 100,
                .joystick_axes_tuned = false,
            };
        }

    private:
        static constexpr AxisInput::Config axisDefaults(bool inverted) noexcept {
            return AxisInput::Config{
                .inverted = inverted,
                .dead_zone = 200,
                .range_positive = 2000,
                .range_negative = 2000,
            };
        }
    };

    explicit Periphery(const Config &config) noexcept :
        config{config} {}

    const Config &config;

    ButtonListener left_button_listener{
        config.button,
        GPIO::DigitalInput{
            gpio_button_left,
            GPIO::DigitalInput::Pull::InternalUp,
        },
    };

    ButtonListener right_button_listener{
        config.button,
        GPIO::DigitalInput{
            gpio_button_right,
            GPIO::DigitalInput::Pull::InternalUp,
        },
    };

    Joystick left_joystick{
        config.left_joystick,
        config.axis_filter,
        GPIO::AdcInput{gpio_joystick_left_x},
        GPIO::AdcInput{gpio_joystick_left_y},
    };

    Joystick right_joystick{
        config.right_joystick,
        config.axis_filter,
        GPIO::AdcInput{gpio_joystick_right_x},
        GPIO::AdcInput{gpio_joystick_right_y},
    };

    SpiBus spi_bus{
        config.spi_bus,
        SPI,
    };

    IicBus iic_bus{
        config.iic_bus,
        Wire,
    };

    DisplayDriver display_driver{

#if defined(DJC_DISPLAY_DRIVER_ST7735)

        config.st7735,
        spi_bus.createNode(config.st7735_spi_node),
        GPIO::DigitalOutput{gpio_display_st7735_data_command},
        GPIO::DigitalOutput{gpio_display_st7735_reset},

#elif defined(DJC_DISPLAY_DRIVER_SSD1306)

        iic_bus.createNode(config.ssd1306_iic_node),

#else

#endif

    };

    // Analog axis calibration
    void tune(Config &mutable_config) noexcept {
        Joystick::Tuner left_tuner{mutable_config.left_joystick, left_joystick, mutable_config.joystick_axes_tune_samples};
        Joystick::Tuner right_tuner{mutable_config.right_joystick, right_joystick, mutable_config.joystick_axes_tune_samples};

        left_tuner.reset();
        right_tuner.reset();

        // Poll all axes until calibration complete
        while (left_tuner.running() or right_tuner.running()) {
            left_tuner.poll();
            right_tuner.poll();
            delay(1);
        }

        mutable_config.joystick_axes_tuned = true;
    }

private:
    static constexpr auto logger = kf::Logger::create("Periphery");

    KF_IMPL_INITABLE(Periphery, void());
    void initImpl() noexcept {
        logger.info("Initializing peripherals");

        left_joystick.init();
        right_joystick.init();
        left_button_listener.init();
        right_button_listener.init();

        if (spi_bus.init().isError()) {
            logger.error("SPI bus init failed");
        }

        if (iic_bus.init().isError()) {
            logger.error("I2C bus init failed");
        }

        if (display_driver.init().isError()) {
            logger.error("Display driver init failed");
        }
    }
};

}// namespace djc
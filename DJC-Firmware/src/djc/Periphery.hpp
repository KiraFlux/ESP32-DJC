#pragma once

// Define flag than djc periphery now using legacy (SSD1306 display driver) instead of ST7735
// #define DJC_USE_LEGACY_DISPLAY_DRIVER

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/core/attributes.hpp>
#include <kf/drivers/input/Button.hpp>
#include <kf/drivers/input/Joystick.hpp>
#include <kf/network/EspNow.hpp>
#include <kf/drivers/input/JoystickListener.hpp>


#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER

#include <kf/drivers/display/SSD1306.hpp>


#else

#include <kf/drivers/display/ST7735.hpp>


#endif

namespace djc {

/// @brief ESP32-DJC Hardware / Periphery
struct Periphery {

    struct Config {

        kf::EspNow::Mac peer_mac{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

        // input

        kf::Button::Config left_button{
            GPIO_NUM_14,
            kf::Button::Config::Mode::PullUp,
            kf::Button::Config::PullType::Internal,
        };

        kf::Joystick::Config left_joystick{
            .x={
                GPIO_NUM_32,
                kf::AnalogAxis::Config::Mode::Inverted,
            },
            .y={
                GPIO_NUM_33,
                kf::AnalogAxis::Config::Mode::Normal,
            }
        };

        kf::Button::Config right_button{
            GPIO_NUM_4,
            kf::Button::Config::Mode::PullUp,
            kf::Button::Config::PullType::Internal,
        };

        kf::Joystick::Config right_joystick{
            .x={
                GPIO_NUM_35,
                kf::AnalogAxis::Config::Mode::Normal,
            },
            .y={
                GPIO_NUM_34,
                kf::AnalogAxis::Config::Mode::Inverted,
            }
        };

        // display driver
#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER
        // SDA: 21
        // SCL: 22
        kf::SSD1306::Config display{
            1000000u, // clock
        };
#else
        // MOSI: 23
        // MISO: 19
        // SCK : 18
        kf::ST7735::Config display{
            // pins: spi_cs, dc, reset
            GPIO_NUM_5, GPIO_NUM_2, GPIO_NUM_15,
            27000000, // SPI: frequency
            kf::ST7735::Orientation::ClockWise,
        };
#endif

    };

    Config config{};

    /// @brief Left Button
    /// @note Uses to switch between modes
    kf::Button left_button{config.left_button};

    /// @brief Left X-Y Stick 
    kf::Joystick left_joystick{config.left_joystick, 0.5f};

    //

    /// @brief Right Button
    /// @note Uses in UI context as 'click' input.
    kf::Button right_button{config.right_button};

    /// @brief Right X-Y Stick
    kf::Joystick right_joystick{config.right_joystick, 0.5f};

    /// @brief Right joystick discrete input listener
    kf::JoystickListener right_joystick_listener{right_joystick};

    //

    /// @brief ESPNOW Peer
    kf::Option<kf::EspNow::Peer> espnow_peer{};

    //

#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER
    using SelectedDisplayDriver = kf::SSD1306;
    SelectedDisplayDriver display{config.display, Wire};
#else
    using SelectedDisplayDriver = kf::ST7735;
    SelectedDisplayDriver display{config.display, SPI};
#endif

    /// @brief Initialize all periphery
    /// @returns true - init ok
    kf_nodiscard bool init() noexcept {
        kf_Logger_info("start");

        if (not display.init()) {
            kf_Logger_error("Display driver init fail");
        }

        left_joystick.init();
        right_joystick.init();
        left_button.init();
        right_button.init();

        const auto espnow_init_result = kf::EspNow::init();
        if (not espnow_init_result.isOk()) {
            kf_Logger_error(
                "Failed to initialize ESP-NOW: %s",
                kf::EspNow::stringFromError(espnow_init_result.error().value()));
            return false;
        }

        const auto peer_result = kf::EspNow::Peer::add(config.peer_mac);

        if (peer_result.isError()) {
            kf_Logger_error(
                "Espnow failed to add peer '%s': %s",
                kf::EspNow::stringFromMac(config.peer_mac),
                kf::EspNow::stringFromError(peer_result.error().value()));
            return false;
        }

        espnow_peer = peer_result.ok();

        kf_Logger_info("OK");
        return true;
    }
};

}// namespace djc

// keep clean
#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER
#undef DJC_USE_LEGACY_DISPLAY_DRIVER
#endif
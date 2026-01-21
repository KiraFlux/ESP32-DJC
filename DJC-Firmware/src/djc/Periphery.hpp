#pragma once

// Define flag than djc periphery now using legacy (SSD1306 display driver) instead of ST7735
//#define DJC_USE_LEGACY_DISPLAY_DRIVER

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/pattern/Singleton.hpp>
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

/// @brief Аппаратное обеспечение пульта
struct Periphery : kf::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    // Periphery config
    struct Config {

#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER
        // SDA: 21
        // SCL: 22
        kf::SSD1306::Config display{
            1000000u, // clock
        };
#else
        //MOSI: 23
        //MISO: 19
        //SCK : 18
        kf::ST7735::Config display{
            GPIO_NUM_5,// SPI: cs
            GPIO_NUM_2, // dc
            GPIO_NUM_15, // reset
            27000000, // SPI: frequency
            kf::ST7735::Orientation::ClockWise,
        };
#endif

    };

    Config config{};

    /// @brief Кнопка левого стика
    /// @details Используется для смены режима
    kf::Button left_button{GPIO_NUM_14, kf::Button::Mode::PullUp};

    /// @brief Левый X-Y Джойстик
    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};

    //

    /// @brief Кнопка правого стика
    /// @details Произвольное использование в пользовательских режимах
    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};

    /// @brief Правый X-Y Джойстик
    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};

    /// @brief Обработчик дискретного ввода джойстика
    kf::JoystickListener right_joystick_listener{right_joystick};

    //

    /// @brief Пир ESPNOW
    kf::Option<kf::EspNow::Peer> espnow_peer;

    //

#ifdef DJC_USE_LEGACY_DISPLAY_DRIVER
    kf::SSD1306 display{config.display, Wire};
#else
    kf::ST7735 display{config.display, SPI};
#endif

    /// @brief Процедура инициализации аппаратных компонентов
    /// @returns true - Успешная инициализация всех аппаратных компонентов
    kf_nodiscard bool init() {
        kf_Logger_info("init");

        if (not display.init()) {
            kf_Logger_error("Display driver init fail");
        }

        display.send();

        left_joystick.init();
        right_joystick.init();
        left_button.init(kf::Button::PullType::Internal);
        right_button.init(kf::Button::PullType::Internal);

        const auto espnow_init_result = kf::EspNow::init();
        if (not espnow_init_result.isOk()) {
            kf_Logger_error(
                "Failed to initialize ESP-NOW: %s",
                kf::EspNow::stringFromError(espnow_init_result.error().value()));
            return false;
        }
        const kf::EspNow::Mac peer_mac{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        const auto peer_result = kf::EspNow::Peer::add(peer_mac);

        if (not peer_result.isOk()) {
            kf_Logger_error(
                "Espnow failed to add peer '%s': %s",
                kf::EspNow::stringFromMac(peer_mac),
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
#pragma once

#include <kf/Logger.hpp>
#include <kf/Option.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include <kf/Button.hpp>
#include <kf/Joystick.hpp>
#include <kf/JoystickListener.hpp>
#include <kf/SSD1306.hpp>
#include <kf/EspNow.hpp>


namespace djc {

/// @brief Аппаратное обеспечение пульта
struct Periphery : kf::tools::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    /// @brief Кнопка левого стика
    /// @details Используется для смены режима
    kf::Button left_button{GPIO_NUM_15, kf::Button::Mode::PullUp};

    /// @brief Левый X-Y Джойстик
    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};

    //

    /// @brief Кнопка правого стика
    /// @details Произвольное использование в пользовательских режимах
    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};

    /// @brief Правый X-Y Джойстик
    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};

    /// @brief Обработчик дискретного ввода джойстика
    kf::JoystickListener joystick_listener{right_joystick};

    //

    /// @brief Пир ESPNOW
    kf::Option<kf::EspNow::Peer> espnow_peer;

    //

    /// @brief Экран пульта
    kf::SSD1306 display_driver{};

    /// @brief Процедура инициализации аппаратных компонентов
    /// @returns true - Успешная инициализация всех аппаратных компонентов
    [[nodiscard]] bool init() {
        kf_Logger_info("init");

        if (not display_driver.init()) {
            kf_Logger_error("Screen driver error");
//            return false;
        }

        Wire.setClock(1000000u);
        display_driver.flush();

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

    /// @brief Процедура конфигурации
    void configure() {
        left_joystick.axis_x.inverted = true;
        right_joystick.axis_y.inverted = true;
    }

    /// @brief Процедура блокирующей калибровки
    /// @param joystick_samples
    void calibrate(int joystick_samples = 500) {
        left_joystick.calibrate(joystick_samples);
        right_joystick.calibrate(joystick_samples);
    }
};

}// namespace djc

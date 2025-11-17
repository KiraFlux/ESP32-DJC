#pragma once

#include <kf/Button.hpp>
#include <kf/Joystick.hpp>
#include <kf/JoystickListener.hpp>
#include <kf/Logger.hpp>
#include <kf/SSD1306.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/remote/EspnowNode.hpp"

namespace djc {

/// @brief Аппаратное обеспечение пульта
struct Periphery : kf::tools::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    /// @brief Кнопка левого стика
    kf::Button left_button{GPIO_NUM_15, kf::Button::Mode::PullUp};

    /// @brief Левый X-Y Джойстик
    kf::Joystick left_joystick{GPIO_NUM_32, GPIO_NUM_33, 0.5f};

    /// @brief Обработчик дискретного ввода левого джойстика // todo Перенести в behavior
    kf::JoystickListener left_joystick_listener{left_joystick};

    //

    /// @brief Кнопка правого стика
    kf::Button right_button{GPIO_NUM_4, kf::Button::Mode::PullUp};

    /// @brief Правый X-Y Джойстик
    kf::Joystick right_joystick{GPIO_NUM_35, GPIO_NUM_34, 0.5f};

    //

    /// @brief Пир ESPNOW
    EspnowNode espnow_node{{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    //

    /// @brief Экран пульта
    kf::SSD1306 screen_driver{};

    /// @brief Процедура инициализации аппаратных компонентов
    /// @returns true - Успешная инициализация всех аппаратных компонентов
    [[nodiscard]] bool init() {
        kf_Logger_info("init");

        if (not screen_driver.init()) {
            kf_Logger_error("Screen driver error");
            return false;
        }

        Wire.setClock(1000000u);
        screen_driver.flush();

        left_joystick.init();
        right_joystick.init();
        left_button.init(kf::Button::PullType::Internal);
        right_button.init(kf::Button::PullType::Internal);

        if (not espnow_node.init()) { return false; }

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

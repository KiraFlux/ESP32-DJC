#pragma once

#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"
#include "djc/behaviors/MavLinkControl.hpp"
#include "djc/behaviors/RemoteUI.hpp"
#include "djc/behaviors/SimpleControl.hpp"
#include "djc/behaviors/LocalUI.hpp"
//#include "djc/behaviors/PongGame.hpp"

namespace djc {

/// @brief Описание поведения пульта
struct RemoteController : kf::sys::BehaviorSystem, kf::tools::Singleton<RemoteController> {
    friend struct Singleton<RemoteController>;

    explicit RemoteController() :
        RemoteController{Periphery::instance().display_driver} {}

    void init() override {
        auto &periphery = Periphery::instance();

        (void) periphery.init();

        BehaviorSystem::init();
        periphery.display_driver.flush();

        periphery.configure();
        periphery.left_button.handler = [this]() { next(); };

        periphery.calibrate();
    }

    /// @brief Отображение системы на дисплей
    void display() override {
        auto &periphery = Periphery::instance();
        //
        BehaviorSystem::display();
        periphery.display_driver.flush();
    }

    /// @brief Обновление системы + пул событий периферии
    void update() override {
        auto &periphery = Periphery::instance();
        //
        periphery.left_button.poll();
        BehaviorSystem::update();
    }

private:
    explicit RemoteController(kf::SSD1306 &display_driver) :
        kf::sys::BehaviorSystem{
            kf::gfx::Canvas{
                kf::gfx::FrameView{
                    display_driver.buffer, display_driver.width(),
                    display_driver.width(), display_driver.height(), 0, 0
                }
            },
            {
                &LocalUI::instance(),
                &MavLinkControl::instance(),
//                &PongGame::instance(),
                &SimpleControl::instance(),
                &RemoteUI::instance(),
            }
        } {}
};

}// namespace djc

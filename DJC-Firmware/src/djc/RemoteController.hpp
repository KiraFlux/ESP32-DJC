#pragma once

#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"
#include "djc/behaviors/SimpleControl.hpp"
#include "djc/behaviors/RemoteInterface.hpp"
#include "djc/behaviors/MavLinkControl.hpp"


namespace djc {

struct RemoteController : kf::sys::BehaviorSystem, kf::tools::Singleton<RemoteController> {
    friend struct Singleton<RemoteController>;

    explicit RemoteController() :
        RemoteController{Periphery::instance().screen_driver} {}

    void init() override {
        auto &periphery = Periphery::instance();
        //
        (void) periphery.init();

        BehaviorSystem::init();
        periphery.screen_driver.flush();

        periphery.configure();
        periphery.right_button.handler = [this]() { next(); };

        periphery.calibrate();
    }

    void display() override {
        auto &periphery = Periphery::instance();
        //
        BehaviorSystem::display();
        periphery.screen_driver.flush();
    }

    void loop() override {
        auto &periphery = Periphery::instance();
        //
        periphery.right_button.poll();
        BehaviorSystem::loop();
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
                &MavLinkControl::instance(),
                &SimpleControl::instance(),
                &RemoteInterface::instance(),
            }
        } {}
};

}

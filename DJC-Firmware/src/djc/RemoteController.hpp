#pragma once

#include <kf/sys.hpp>

#include "djc/Periphery.hpp"
#include "djc/behaviors/FlightControl.hpp"
#include "djc/behaviors/RemoteInterface.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct RemoteController : kf::sys::BehaviorSystem, Singleton<RemoteController> {
    friend struct Singleton<RemoteController>;

    explicit RemoteController() :
        kf::sys::BehaviorSystem{
            kf::Painter{
                kf::FrameView{
                    Periphery::instance().display_driver.buffer, kf::SSD1306::width,
                    kf::SSD1306::width, kf::SSD1306::height, 0, 0
                }
            },
            {
                &FlightControl::instance(),
                &RemoteInterface::instance(),
            }
        } {}

    void init() override {
        auto &periphery = Periphery::instance();
        //
        (void) periphery.init();

        BehaviorSystem::init();
        periphery.display_driver.update();

        periphery.configure();
        periphery.right_button.handler = [this]() { next(); };

        periphery.calibrate();
    }

    void display() override {
        auto &periphery = Periphery::instance();
        //
        BehaviorSystem::display();
        periphery.display_driver.update();
    }

    void loop() override {
        auto &periphery = Periphery::instance();
        //
        periphery.right_button.poll();
        BehaviorSystem::loop();
    }
};

}

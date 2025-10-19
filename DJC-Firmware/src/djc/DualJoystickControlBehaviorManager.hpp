#pragma once

#include <kf/sys.hpp>

#include "djc/Periphery.hpp"
#include "djc/behaviors/FlightBehavior.hpp"
#include "djc/behaviors/RemoteMenuBehavior.hpp"
#include "djc/tools/Singleton.hpp"


namespace djc {

struct DualJoystickControlBehaviorManager : kf::sys::BehaviorManager, Singleton<DualJoystickControlBehaviorManager> {
    friend struct Singleton<DualJoystickControlBehaviorManager>;

    DualJoystickControlBehaviorManager() :
        kf::sys::BehaviorManager{
            kf::Painter{
                kf::FrameView{
                    djc::Periphery::instance().display_driver.buffer, kf::SSD1306::width,
                    kf::SSD1306::width, kf::SSD1306::height, 0, 0
                }
            },
            {
                &djc::FlightBehavior::instance(),
                &djc::RemoteMenuBehavior::instance(),
            }
        } {}

    void init() override {
        auto &periphery = djc::Periphery::instance();
        //
        (void) periphery.init();

        BehaviorManager::init();
        periphery.display_driver.update();

        periphery.configure();
        periphery.right_button.handler = [this]() { next(); };

        periphery.calibrate();
    }

    void display() override {
        auto &periphery = djc::Periphery::instance();
        //
        BehaviorManager::display();
        periphery.display_driver.update();
    }

    void loop() override {
        auto &periphery = djc::Periphery::instance();
        //
        periphery.right_button.poll();
        BehaviorManager::loop();
    }
};

}

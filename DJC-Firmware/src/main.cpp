#include <Arduino.h>

#include <rs/aliases.hpp>
#include <kf/sys.hpp>

#include "djc/Periphery.hpp"
#include "djc/behaviors/FlightBehavior.hpp"
#include "djc/behaviors/RemoteMenuBehavior.hpp"


static void peripheryConfig(djc::Periphery &periphery) {
    periphery.left_joystick.axis_x.inverted = true;
    periphery.right_joystick.axis_y.inverted = true;

    periphery.left_joystick.calibrate(500);
    periphery.right_joystick.calibrate(500);

    periphery.right_button.handler = []() {
        static std::array<kf::sys::Behavior *, 2> behaviors{
            &djc::RemoteMenuBehavior::instance(),
            &djc::FlightBehavior::instance(),
        };
        static rs::size current{0};

        kf::sys::BehaviorManager::instance().bindBehavior(*behaviors[current]);
        kf_Logger_debug("Behavior: %d", current);

        current += 1;
        current %= behaviors.size();
    };
}

void setup() {
    static auto &periphery = djc::Periphery::instance();
    static auto &manager = kf::sys::BehaviorManager::instance();
    static auto &flight_behavior = djc::FlightBehavior::instance();
    static auto &remote_menu_behavior = djc::RemoteMenuBehavior::instance();

    Serial.begin(115200);
    kf::Logger::instance().write_func = [](const char *buffer, rs::size size) {
        Serial.write(buffer, size);
    };

    periphery.init();

    static kf::Painter painter{
        kf::FrameView{
            periphery.display_driver.buffer, kf::SSD1306::width,
            kf::SSD1306::width, kf::SSD1306::height, 0, 0
        },
        kf::fonts::gyver_5x7_en
    };

    flight_behavior.bindPainters(painter);
    remote_menu_behavior.bindPainters(painter);

    painter.text("Init");
    periphery.display_driver.update();

    peripheryConfig(periphery);

    manager.bindBehavior(flight_behavior);

    while (true) {
        manager.loop();
        periphery.right_button.poll();

        manager.display(painter);
        periphery.display_driver.update();

        delay(20);
    }
}

void loop() {}
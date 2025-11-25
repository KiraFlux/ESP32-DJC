#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/tools/time/Timer.hpp>

#include "djc/RemoteController.hpp"


static auto &behavior_manager = djc::RemoteController::instance();

void setup() {
    Serial.begin(115200);
    kf_Logger_setWriter([](const kf::slice<const char> &str) { Serial.write(str.data(), str.size()); });
    behavior_manager.init();
}

void loop() {
    static kf::tools::Timer update_timer{static_cast<kf::Hertz>(50)};
    if (update_timer.ready()) {
        behavior_manager.update();
    }

    static kf::tools::Timer display_timer{static_cast<kf::Hertz>(20)};
    if (display_timer.ready()) {
        behavior_manager.display();
    }

    delay(1);
}

#include <Arduino.h>

#include "djc/DualJoystickControlBehaviorManager.hpp"


static djc::DualJoystickControlBehaviorManager behavior_manager{};

void setup() {
    Serial.begin(115200);
    kf::Logger::instance().write_func = [](const char *buffer, size_t size) { Serial.write(buffer, size); };
    behavior_manager.init();
}

void loop() {
    behavior_manager.loop();
    behavior_manager.display();
    delay(20);
}

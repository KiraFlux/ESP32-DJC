#include <Arduino.h>
#include <kf/Logger.hpp>

#include "djc/RemoteController.hpp"

static auto &behavior_manager = djc::RemoteController::instance();

void setup() {
    Serial.begin(115200);
    kf::Logger::instance().writer = [](const char *buffer, size_t size) { Serial.write(buffer, size); };
    behavior_manager.init();
}

void loop() {
    behavior_manager.loop();
    behavior_manager.display();
    delay(20);
}

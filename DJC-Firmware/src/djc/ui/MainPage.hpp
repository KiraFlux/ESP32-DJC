#pragma once

#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/math/time/Timer.hpp>

#include "djc/UI.hpp"
#include "djc/Periphery.hpp"


namespace djc {

struct MainPage : UI::Page {

    explicit MainPage() :
        Page{"ESP32-DJC"} {}
};

}
#pragma once

#include <kf/pattern/Singleton.hpp>

#include "djc/UI.hpp"
#include "djc/Periphery.hpp"


namespace djc {

struct MainPage : UI::Page, kf::Singleton<MainPage> {
    friend struct Singleton<MainPage>;

    explicit MainPage() :
        Page{"ESP32-DJC"} {}
};

}
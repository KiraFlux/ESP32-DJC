#pragma once

#include <kf/pattern/Singleton.hpp>

#include "djc/UI.hpp"


namespace djc {

/// @brief Main menu page for ESP32-DJC
struct MainPage : UI::Page, kf::Singleton<MainPage> {
    friend struct Singleton<MainPage>;

    explicit MainPage() :
        Page{"ESP32-DJC"} {}
};

} // namespace djc
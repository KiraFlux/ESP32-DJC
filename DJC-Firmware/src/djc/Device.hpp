// // Copyright (c) 2026 KiraFlux
// // SPDX-License-Identifier: MIT

// #pragma once

// #include <kf/gfx/Canvas.hpp>
// #include <kf/image/DynamicImage.hpp>
// #include <kf/mixin/NonCopyable.hpp>
// #include <kf/mixin/TimedPollable.hpp>

// #include "djc/Periphery.hpp"
// #include "djc/ui/UI.hpp"

// namespace djc {

// /// @brief Main device controller for ESP32-DJC
// struct Device : kf::mixin::NonCopyable, kf::mixin::TimedPollable<Device> {


//     // Input event handlers
//     void onLeftButtonClick() noexcept {
//         _menu_navigation_enabled = not _menu_navigation_enabled;
//         _controller_values.reset();
//         ui.addEvent(ui::UI::Event::update());
//     }

//     void onNavigationRightButtonClick() const noexcept {
//         ui.addEvent(ui::UI::Event::widgetClick());
//     }

//     void onNavigationRightJoystickDirection(JoystickListener::Direction direction) const noexcept {
//         static constexpr ui::UI::Event event_from_direction[4] = {
//             ui::UI::Event::pageCursorMove(-1),// Up
//             ui::UI::Event::pageCursorMove(+1),// Down
//             ui::UI::Event::widgetValue(-1),   // Left
//             ui::UI::Event::widgetValue(+1),   // Right
//         };

//         ui.addEvent(event_from_direction[static_cast<kf::u8>(direction)]);
//     }
// };

// }// namespace djc
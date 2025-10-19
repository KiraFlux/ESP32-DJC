#pragma once

// KiraFlux-SYS Entry

/// KiraFlux Graphics Behavior Management System
namespace kf::sys {}

// Abstract Base Classes
#include <kf/sys/abc/Component.hpp>

// Core Components
#include <kf/sys/core/Behavior.hpp>
#include <kf/sys/core/BehaviorSystem.hpp>

// Built-In ABC Implementations
#include <kf/sys/impl/TextComponent.hpp>
#include <kf/sys/impl/JoystickComponent.hpp>
#include <kf/sys/impl/FlagComponent.hpp>

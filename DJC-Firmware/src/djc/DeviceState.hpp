// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/mixin/NonCopyable.hpp>

namespace djc {

struct DeviceState final : kf::mixin::NonCopyable {
    bool menu_navigation_enabled;    
};

}// namespace djc
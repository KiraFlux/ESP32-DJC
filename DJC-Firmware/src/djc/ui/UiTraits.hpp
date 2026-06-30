// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/ui/UiTraits.hpp>

namespace djc::ui {

struct UiTraitsTag {};

/// @brief ESP32-DJC extended UI Traits specialization
/// @tparam W Widget base class
/// @note Any Widget from `djc::widgets` should be noted that `<U>` implements `djc::ui::UiTraits`
template<typename W> struct UiTraits :

    UiTraitsTag,
    kf::ui::UiTraits<W>

{};

}// namespace djc::ui
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

namespace djc::service {

struct ServiceTag {};

/// @brief Service Static inteface, provides NonCopyable & Pollable mixins
template<typename Impl> struct Service :

    ServiceTag,
    kf::mixin::NonCopyable,
    kf::mixin::TimedPollable<Impl> {};

}// namespace djc::service
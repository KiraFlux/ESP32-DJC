// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

namespace djc::system {

struct SystemTag {};

/// @brief System static inteface
/// @note System owns a group of related services and components, provides `init()` and `poll(now)`, and is orchestrated from main.
template<typename Impl, typename InitSignature> struct System :

    SystemTag,
    kf::mixin::NonCopyable,
    kf::mixin::Initable<Impl, InitSignature>,
    kf::mixin::TimedPollable<Impl> {};

}// namespace djc::system
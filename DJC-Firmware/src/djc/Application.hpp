// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/math/units.hpp>
#include <kf/mixin/Initable.hpp>
#include <kf/mixin/NonCopyable.hpp>
#include <kf/mixin/TimedPollable.hpp>

#include "djc/prelude.hpp"
#include "djc/Periphery.hpp"

namespace djc {

struct Application final : kf::mixin::Initable<Application, bool>, kf::mixin::NonCopyable, kf::mixin::TimedPollable<Application> {

private:



    // impl

    KF_IMPL_INITABLE(Application, bool);
    bool initImpl() noexcept {
        return true;
    }

    KF_IMPL_TIMED_POLLABLE(Application);
    void pollImpl(kf::math::Milliseconds now) noexcept {
    }
};

}// namespace djc
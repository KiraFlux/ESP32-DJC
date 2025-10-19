#pragma once

#include <kf/gfx.hpp>


namespace kf::sys {

/// System Component
struct Component {
    gfx::Painter painter{};

    /// Show Component
    virtual void display() = 0;
};

}
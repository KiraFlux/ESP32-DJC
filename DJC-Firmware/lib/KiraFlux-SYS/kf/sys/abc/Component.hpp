#pragma once

#include <kf/Painter.hpp>


namespace kf::sys {

/// System Component
struct Component {
    kf::Painter painter{};

    /// Show Component
    virtual void display() = 0;
};

}
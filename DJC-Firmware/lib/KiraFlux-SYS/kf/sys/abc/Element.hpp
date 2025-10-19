#pragma once

#include "kf/Painter.hpp"


namespace kf::sys {

struct Element {
    kf::Painter painter{};

    virtual void display() = 0;
};

}
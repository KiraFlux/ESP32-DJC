#pragma once

#include <vector>
#include "kf/sys/abc/Element.hpp"


namespace kf::sys {

struct Behavior {

private:
    std::vector<Element *> elements{};

public:
    void add(Element &element) {
        elements.push_back(&element);
    }

    void display() {
        for (auto element: elements) {
            element->display();
        }
    }

    virtual void bindPainters(kf::Painter &root) = 0;

    virtual void loop() = 0;

    virtual void onBind() {}
};


}
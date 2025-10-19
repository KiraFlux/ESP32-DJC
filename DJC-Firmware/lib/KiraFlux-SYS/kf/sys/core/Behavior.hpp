#pragma once

#include <vector>
#include "kf/sys/abc/Element.hpp"


namespace kf::sys {

struct Behavior {

private:
    std::vector<Element *> elements{};

public:
    void add(Element &widget) {
        elements.push_back(&widget);
    }

    void display() {
        for (auto w: elements) {
            w->display();
        }
    }

    virtual void bindPainters(kf::Painter &root) {};

    virtual void onBind() {}

    virtual void loop() {}
};


}
#pragma once

#include <vector>

#include <kf/sys/abc/Component.hpp>


namespace kf::sys {

struct Behavior {

private:
    std::vector<Component *> components{};

public:
    void addComponent(Component &component) {
        components.push_back(&component);
    }

    void display() {
        for (auto component: components) {
            component->display();
        }
    }

    virtual void updateLayout(gfx::Canvas &root) = 0;

    virtual void update() = 0;

    virtual void onEntry() {}
};

}
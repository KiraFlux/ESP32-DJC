#pragma once

#include <vector>
#include <rs/aliases.hpp>
#include <kf/gfx.hpp>

#include <kf/sys/core/Behavior.hpp>


namespace kf::sys {

struct BehaviorSystem {

private:
    std::vector<Behavior *> behaviors;
    gfx::Canvas root_canvas{};
    rs::size cursor{0};

public:
    explicit BehaviorSystem(
        const gfx::Canvas &root,
        std::initializer_list<Behavior *> behaviors
    ) :
        behaviors{behaviors}, root_canvas{root} {
        root_canvas.setFont(gfx::fonts::gyver_5x7_en);

        for (auto b: behaviors) {
            b->updateLayout(root_canvas);
        }
    }

    virtual void init() {
        root_canvas.text("Initializing...");
        auto behavior = getCurrentBehavior();
        if (nullptr == behavior) { return; }
        behavior->onEntry();
    }

    virtual void display() {
        auto behavior = getCurrentBehavior();
        if (nullptr == behavior) { return; }
        root_canvas.fill(false);
        behavior->display();
    }

    virtual void loop() {
        auto behavior = getCurrentBehavior();
        if (nullptr == behavior) { return; }
        behavior->update();
    }

    void next() {
        if (behaviors.empty()) { return; }

        cursor += 1;
        cursor %= behaviors.size();

        behaviors[cursor]->onEntry();
    }

private:
    Behavior *getCurrentBehavior() { return behaviors.empty() ? nullptr : behaviors[cursor]; }
};

}
#pragma once

#include <KiraFlux-GFX.hpp>
#include <rs/aliases.hpp>
#include <vector>

#include <kf/sys/core/Behavior.hpp>


namespace kf::sys {

struct BehaviorManager {

private:
    std::vector<Behavior *> behaviors;
    Painter root_canvas{};
    rs::size cursor{0};

public:
    explicit BehaviorManager(
        const Painter &root,
        std::initializer_list<Behavior *> behaviors
    ) :
        behaviors{behaviors}, root_canvas{root} {
        for (auto b: behaviors) {
            b->bindPainters(root_canvas);
        }
    }

    virtual void init() {
        root_canvas.setFont(kf::fonts::gyver_5x7_en);
        root_canvas.text("Initializing...");
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
        behavior->loop();
    }

    void next() {
        if (behaviors.empty()) { return; }

        cursor += 1;
        cursor %= behaviors.size();

        behaviors[cursor]->onBind();
    }

private:
    Behavior *getCurrentBehavior() { return behaviors.empty() ? nullptr : behaviors[cursor]; }
};

}
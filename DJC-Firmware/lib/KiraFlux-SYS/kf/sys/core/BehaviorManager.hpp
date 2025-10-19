#pragma once

#include "Behavior.hpp"


namespace kf::sys {

/// Менеджер поведения
struct BehaviorManager final {

private:

    /// Активное поведение
    Behavior *active_behavior{nullptr};

public:

    static BehaviorManager &instance() {
        static BehaviorManager instance{};
        return instance;
    }

    [[nodiscard]] inline bool isActive(const Behavior &behavior) const {
        return &behavior == active_behavior;
    }

    void bindBehavior(Behavior &behavior) {
        active_behavior = &behavior;
        behavior.onBind();
    }

    void display(kf::Painter &painter) {
        if (active_behavior != nullptr) {
            painter.fill(false);
            active_behavior->display();
        }
    }

    void loop() {
        if (active_behavior != nullptr) {
            active_behavior->loop();
        }
    }

private:
    BehaviorManager() = default;

public:
    BehaviorManager(const BehaviorManager &) = delete;
};


}
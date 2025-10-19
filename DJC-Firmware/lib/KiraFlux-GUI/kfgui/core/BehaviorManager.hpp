#pragma once

#include "kfgui/core/Behavior.hpp"


namespace kfgui {

/// Менеджер поведения
struct BehaviorManager final {

private:

    /// Активное поведение
    Behavior *active_behavior{nullptr};

public:

    static BehaviorManager &instance() noexcept {
        static BehaviorManager instance{};
        return instance;
    }

    [[nodiscard]] inline bool isActive(const Behavior &behavior) const noexcept {
        return &behavior == active_behavior;
    }

    void bindBehavior(Behavior &behavior) noexcept {
        active_behavior = &behavior;
        behavior.onBind();
    }

    void render(kf::Painter &painter) noexcept {
        if (active_behavior != nullptr) {
            painter.fill(false);
            active_behavior->render();
        }
    }

    void loop() noexcept {
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
#pragma once
#include <core/Containers.hpp>
#include "SnakeGame.hpp"
#include <scene/Component.hpp>
#include <array>

namespace ursa::demo {
class SnakeController final : public Component {
public:
    SnakeController(SnakeGame &game, bool stressWindow, bool smokeGameplay);

    Result<void> onUpdate(const FrameContext &context) override;

    Result<void> onFixedUpdate(const FrameContext &context) override;

    bool stressWindow() const {
        return stressWindow_;
    }

    bool smokeGameplay() const {
        return smokeGameplay_;
    }

    void setStressWindow(bool value) {
        stressWindow_ = value;
    }

    void setSmokeGameplay(bool value) {
        smokeGameplay_ = value;
    }

private:
    Result<void> runSmokeStep();

    void readInput(const FrameContext &context);

    SnakeGame &game_;
    FixedStepClock clock_;
    ursa::Array<bool, 8> previousKeys_{};
    uint32_t frame_ = 0;
    bool stressWindow_ = false;
    bool smokeGameplay_ = false;
    bool previousSpace_ = false;
};
} // namespace ursa::demo

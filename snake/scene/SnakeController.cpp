#include <core/Containers.hpp>
#include "SnakeController.hpp"
#include <array>

namespace ursa::demo {
SnakeController::SnakeController(SnakeGame &game, bool stressWindow, bool smokeGameplay)
    : game_(game), stressWindow_(stressWindow), smokeGameplay_(smokeGameplay) {
}

Result<void> SnakeController::onUpdate(const FrameContext &context) {
    ++frame_;
    if (stressWindow_ && frame_ == 5)
        context.window->resize(800, 600);
    if (stressWindow_ && frame_ == 10) {
        context.window->minimize();
        context.window->restore();
    }
    if (context.input.keyDown(Key::Escape))
        context.window->requestClose();
    readInput(context);
    if (smokeGameplay_)
        return runSmokeStep();
    return {};
}

Result<void> SnakeController::onFixedUpdate(const FrameContext &context) {
    if (!smokeGameplay_)
        for (int step = 0, count = clock_.advance(context.fixedDeltaSeconds); step < count; ++step)
            game_.step();
    return {};
}

void SnakeController::readInput(const FrameContext &context) {
    const ursa::Array<Key, 8> keys{Key::Up, Key::W, Key::Down, Key::S, Key::Left, Key::A, Key::Right, Key::D};
    const ursa::Array<Direction, 8> directions{Direction::Up,   Direction::Up,   Direction::Down,  Direction::Down,
                                               Direction::Left, Direction::Left, Direction::Right, Direction::Right};
    for (size_t i = 0; i < keys.size(); ++i) {
        const bool down = context.input.keyDown(keys[i]);
        if (down && !previousKeys_[i])
            game_.queueDirection(directions[i]);
        previousKeys_[i] = down;
    }
    const bool space = context.input.keyDown(Key::Space);
    if (space && !previousSpace_) {
        game_.reset();
        clock_.reset();
    }
    previousSpace_ = space;
}

Result<void> SnakeController::runSmokeStep() {
    if (frame_ == 1) {
        game_.step();
        if (game_.score() != 1)
            return Result<void>::failure("Smoke game did not eat initial food");
    } else if (frame_ == 2 || frame_ == 3) {
        game_.step();
        if (frame_ == 3 && game_.status() != GameStatus::Lost)
            return Result<void>::failure("Smoke game did not reach game over");
    } else if (frame_ == 4) {
        game_.reset();
        if (game_.status() != GameStatus::Running || game_.score() != 0)
            return Result<void>::failure("Smoke game did not restart");
    }
    return {};
}
} // namespace ursa::demo

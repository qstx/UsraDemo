#include "SnakeGame.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {
using namespace ursa::demo;

void expect(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void testFixedStep() {
    FixedStepClock slow(0.2);
    FixedStepClock fast(0.2);
    int slowSteps = 0;
    int fastSteps = 0;
    for (int frame = 0; frame < 10; ++frame)
        slowSteps += slow.advance(0.1);
    for (int frame = 0; frame < 20; ++frame)
        fastSteps += fast.advance(0.05);
    expect(slowSteps == 5 && fastSteps == 5, "Movement depends on render frame rate");
    slow.reset();
    expect(slow.advance(0.1) == 0, "Clock reset did not clear partial time");
    expect(fast.advance(2.0) <= 2, "Long frame caused excessive catch-up steps");
}

void testDirectionBuffer() {
    SnakeConfig config;
    config.width = config.height = 8;
    config.initialFood = Cell{7, 7};
    SnakeGame game(config);
    game.queueDirection(Direction::Left);
    game.queueDirection(Direction::Up);
    game.queueDirection(Direction::Down);
    game.step();
    expect(game.direction() == Direction::Up && game.body().front() == Cell{4, 3},
           "Direction buffering or reverse protection failed");
    game.queueDirection(Direction::Down);
    game.step();
    expect(game.direction() == Direction::Up && game.body().front() == Cell{4, 2},
           "Direct reversal changed snake direction");
}

void testFoodAndReset() {
    SnakeConfig config;
    config.width = config.height = 4;
    config.initialBody = {{1, 1}, {0, 1}};
    config.initialFood = Cell{2, 1};
    SnakeGame game(config);
    game.step();
    expect(game.score() == 1 && game.body().size() == 3, "Eating did not grow snake and score");
    expect(game.food() && std::find(game.body().begin(), game.body().end(), *game.food()) == game.body().end(),
           "Food appeared on the snake");
    game.reset();
    expect(game.score() == 0 && game.body().size() == 2 && game.food() == config.initialFood,
           "Restart did not restore the initial state");
}

void testCollisionAndTail() {
    SnakeConfig wall;
    wall.width = wall.height = 4;
    wall.initialBody = {{3, 1}, {2, 1}};
    wall.initialFood = Cell{0, 0};
    SnakeGame wallGame(wall);
    wallGame.step();
    expect(wallGame.status() == GameStatus::Lost && wallGame.body().front() == Cell{3, 1},
           "Wall collision did not stop the game");
    wallGame.reset();
    expect(wallGame.status() == GameStatus::Running, "Restart did not clear the loss state");

    SnakeConfig loop;
    loop.width = loop.height = 4;
    loop.initialBody = {{1, 1}, {1, 2}, {2, 2}, {2, 1}};
    loop.initialDirection = Direction::Up;
    loop.initialFood = Cell{0, 0};
    SnakeGame tailGame(loop);
    tailGame.queueDirection(Direction::Right);
    tailGame.step();
    expect(tailGame.status() == GameStatus::Running && tailGame.body().front() == Cell{2, 1},
           "Moving into the vacated tail should be legal");

    loop.initialBody = {{1, 1}, {1, 2}, {2, 2}, {2, 1}, {3, 1}};
    SnakeGame bodyGame(loop);
    bodyGame.queueDirection(Direction::Right);
    bodyGame.step();
    expect(bodyGame.status() == GameStatus::Lost, "Body collision was not detected");
}

void testWin() {
    SnakeConfig config;
    config.width = config.height = 2;
    config.initialBody = {{0, 0}, {0, 1}, {1, 1}};
    config.initialFood = Cell{1, 0};
    SnakeGame game(config);
    game.step();
    expect(game.status() == GameStatus::Won && !game.food() && game.body().size() == 4 && game.score() == 1,
           "Filling the board did not win cleanly");
    game.step();
    expect(game.body().size() == 4, "Won game continued moving");
}
} // namespace

int main() {
    try {
        testFixedStep();
        testDirectionBuffer();
        testFoodAndReset();
        testCollisionAndTail();
        testWin();
        std::cout << "Snake rules tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Snake rules test failed: " << error.what() << '\n';
        return 1;
    }
}

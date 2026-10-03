#pragma once
#include <core/Containers.hpp>
#include <cstdint>
#include <deque>
#include <optional>
#include <random>
#include <vector>

namespace ursa::demo {
struct Cell {
    int x = 0;
    int z = 0;

    bool operator==(Cell other) const {
        return x == other.x && z == other.z;
    }
};

enum class Direction { Up, Down, Left, Right };
enum class GameStatus { Running, Lost, Won };

struct SnakeConfig {
    int width = 16;
    int height = 16;
    uint32_t seed = 1;
    ursa::Vector<Cell> initialBody;
    Direction initialDirection = Direction::Right;
    std::optional<Cell> initialFood;
};

class FixedStepClock {
public:
    explicit FixedStepClock(double intervalSeconds = 0.18);

    int advance(double deltaSeconds);

    void reset();

private:
    double interval_;
    double accumulated_ = 0.0;
};

class SnakeGame {
public:
    explicit SnakeGame(SnakeConfig config = {});

    void reset();

    void queueDirection(Direction direction);

    void step();

    int width() const;

    int height() const;

    const ursa::Deque<Cell> &body() const;

    std::optional<Cell> food() const;

    Direction direction() const;

    GameStatus status() const;

    int score() const;

private:
    void spawnFood();

    SnakeConfig config_;
    std::mt19937 random_;
    ursa::Deque<Cell> body_;
    std::optional<Cell> food_;
    std::optional<Direction> pendingDirection_;
    Direction direction_ = Direction::Right;
    GameStatus status_ = GameStatus::Running;
    int score_ = 0;
};
} // namespace ursa::demo

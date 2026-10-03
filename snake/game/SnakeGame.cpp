#include <core/Containers.hpp>
#include "SnakeGame.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace ursa::demo {
namespace {
bool opposite(Direction a, Direction b) {
    return (a == Direction::Up && b == Direction::Down) || (a == Direction::Down && b == Direction::Up) ||
           (a == Direction::Left && b == Direction::Right) || (a == Direction::Right && b == Direction::Left);
}

Cell moved(Cell cell, Direction direction) {
    switch (direction) {
    case Direction::Up:
        --cell.z;
        break;
    case Direction::Down:
        ++cell.z;
        break;
    case Direction::Left:
        --cell.x;
        break;
    case Direction::Right:
        ++cell.x;
        break;
    }
    return cell;
}
} // namespace

FixedStepClock::FixedStepClock(double intervalSeconds) : interval_(intervalSeconds) {
    if (intervalSeconds <= 0.0)
        throw std::invalid_argument("Simulation interval must be positive");
}

int FixedStepClock::advance(double deltaSeconds) {
    if (deltaSeconds < 0.0)
        throw std::invalid_argument("Frame time cannot be negative");
    accumulated_ += std::min(deltaSeconds, 0.25);
    int steps = 0;
    while (accumulated_ + 0.000000001 >= interval_) {
        accumulated_ -= interval_;
        ++steps;
    }
    return steps;
}

void FixedStepClock::reset() {
    accumulated_ = 0.0;
}

SnakeGame::SnakeGame(SnakeConfig config) : config_(std::move(config)), random_(config_.seed) {
    if (config_.width < 2 || config_.height < 2)
        throw std::invalid_argument("Board must be at least 2 by 2");
    if (config_.initialBody.empty()) {
        if (config_.width < 4)
            throw std::invalid_argument("Default snake needs a board at least four cells wide");
        const int x = config_.width / 2;
        const int z = config_.height / 2;
        config_.initialBody = {{x, z}, {x - 1, z}, {x - 2, z}};
    }
    for (size_t i = 0; i < config_.initialBody.size(); ++i) {
        const Cell cell = config_.initialBody[i];
        if (cell.x < 0 || cell.x >= config_.width || cell.z < 0 || cell.z >= config_.height ||
            std::find(config_.initialBody.begin(), config_.initialBody.begin() + i, cell) !=
                config_.initialBody.begin() + i)
            throw std::invalid_argument("Initial snake body is outside the board or overlaps itself");
    }
    if (config_.initialFood) {
        const Cell cell = *config_.initialFood;
        if (cell.x < 0 || cell.x >= config_.width || cell.z < 0 || cell.z >= config_.height ||
            std::find(config_.initialBody.begin(), config_.initialBody.end(), cell) != config_.initialBody.end())
            throw std::invalid_argument("Initial food must occupy a free board cell");
    }
    reset();
}

void SnakeGame::reset() {
    random_.seed(config_.seed);
    body_.assign(config_.initialBody.begin(), config_.initialBody.end());
    direction_ = config_.initialDirection;
    pendingDirection_.reset();
    score_ = 0;
    status_ = GameStatus::Running;
    food_ = config_.initialFood;
    if (!food_)
        spawnFood();
}

void SnakeGame::queueDirection(Direction direction) {
    if (status_ != GameStatus::Running || pendingDirection_ || opposite(direction, direction_))
        return;
    pendingDirection_ = direction;
}

void SnakeGame::step() {
    if (status_ != GameStatus::Running)
        return;
    if (pendingDirection_) {
        direction_ = *pendingDirection_;
        pendingDirection_.reset();
    }
    const Cell next = moved(body_.front(), direction_);
    if (next.x < 0 || next.x >= config_.width || next.z < 0 || next.z >= config_.height) {
        status_ = GameStatus::Lost;
        return;
    }
    const bool eating = food_ && next == *food_;
    auto occupiedEnd = body_.end();
    if (!eating)
        --occupiedEnd;
    if (std::find(body_.begin(), occupiedEnd, next) != occupiedEnd) {
        status_ = GameStatus::Lost;
        return;
    }
    body_.push_front(next);
    if (eating) {
        ++score_;
        spawnFood();
    } else {
        body_.pop_back();
    }
}

int SnakeGame::width() const {
    return config_.width;
}

int SnakeGame::height() const {
    return config_.height;
}

const ursa::Deque<Cell> &SnakeGame::body() const {
    return body_;
}

std::optional<Cell> SnakeGame::food() const {
    return food_;
}

Direction SnakeGame::direction() const {
    return direction_;
}

GameStatus SnakeGame::status() const {
    return status_;
}

int SnakeGame::score() const {
    return score_;
}

void SnakeGame::spawnFood() {
    ursa::Vector<Cell> freeCells;
    for (int z = 0; z < config_.height; ++z)
        for (int x = 0; x < config_.width; ++x) {
            const Cell cell{x, z};
            if (std::find(body_.begin(), body_.end(), cell) == body_.end())
                freeCells.push_back(cell);
        }
    if (freeCells.empty()) {
        food_.reset();
        status_ = GameStatus::Won;
        return;
    }
    std::uniform_int_distribution<size_t> distribution(0, freeCells.size() - 1);
    food_ = freeCells[distribution(random_)];
}
} // namespace ursa::demo

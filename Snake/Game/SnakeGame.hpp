#pragma once
#include <Core/Containers.hpp>
#include <cstdint>
#include <deque>
#include <optional>
#include <random>
#include <vector>

namespace Ursa::Demo
{
struct Cell
{
	int x = 0;
	int z = 0;

	bool operator==(Cell other) const
	{
		return x == other.x && z == other.z;
	}
};

enum class Direction
{
	up,
	down,
	left,
	right
};
enum class GameStatus
{
	running,
	lost,
	won
};

struct SnakeConfig
{
	int width = 16;
	int height = 16;
	uint32_t seed = 1;
	Ursa::Vector<Cell> initialBody;
	Direction initialDirection = Direction::right;
	std::optional<Cell> initialFood;
};

class FixedStepClock
{
public:
	explicit FixedStepClock(double intervalSeconds = 0.18);

	int Advance(double deltaSeconds);

	void Reset();

private:
	double interval;
	double accumulated = 0.0;
};

class SnakeGame
{
public:
	explicit SnakeGame(SnakeConfig config = {});

	void Reset();

	void QueueDirection(Direction direction);

	void Step();

	int GetWidth() const;

	int GetHeight() const;

	const Ursa::Deque<Cell>& GetBody() const;

	std::optional<Cell> GetFood() const;

	Direction GetDirection() const;

	GameStatus GetStatus() const;

	int GetScore() const;

private:
	void SpawnFood();

	SnakeConfig config;
	std::mt19937 random;
	Ursa::Deque<Cell> body;
	std::optional<Cell> food;
	std::optional<Direction> pendingDirection;
	Direction direction = Direction::right;
	GameStatus status = GameStatus::running;
	int score = 0;
};
} // namespace Ursa::Demo

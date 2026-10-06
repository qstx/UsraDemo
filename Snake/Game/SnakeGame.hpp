#pragma once
#include <Core/Types.hpp>
#include <Core/Containers.hpp>
#include <cstdint>
#include <optional>
#include <random>

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
	Up,
	Down,
	Left,
	Right
};
enum class GameStatus
{
	Running,
	Lost,
	Won
};

struct SnakeConfig
{
	int width = 16;
	int height = 16;
	Ursa::UInt32 seed = 1;
	Ursa::Vector<Cell> initialBody;
	Direction initialDirection = Direction::Right;
	std::optional<Cell> initialFood;
};

class FixedStepClock
{
public:
	explicit FixedStepClock(Ursa::Float64 intervalSeconds = 0.18);

	int Advance(Ursa::Float64 deltaSeconds);

	void Reset();

private:
	Ursa::Float64 interval;
	Ursa::Float64 accumulated = 0.0;
};

class SnakeGame
{
public:
	explicit SnakeGame(SnakeConfig initialConfig = {});

	void Reset();

	void QueueDirection(Direction requestedDirection);

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
	Direction direction = Direction::Right;
	GameStatus status = GameStatus::Running;
	int score = 0;
};
} // namespace Ursa::Demo

#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace Ursa::Demo
{
namespace
{
bool Opposite(Direction a, Direction b)
{
	return (a == Direction::up && b == Direction::down) || (a == Direction::down && b == Direction::up) ||
	       (a == Direction::left && b == Direction::right) || (a == Direction::right && b == Direction::left);
}

Cell moved(Cell cell, Direction direction)
{
	switch (direction)
	{
		case Direction::up:
			--cell.z;
			break;
		case Direction::down:
			++cell.z;
			break;
		case Direction::left:
			--cell.x;
			break;
		case Direction::right:
			++cell.x;
			break;
	}
	return cell;
}
} // namespace

FixedStepClock::FixedStepClock(double intervalSeconds) : interval(intervalSeconds)
{
	if (intervalSeconds <= 0.0)
	{
		throw std::invalid_argument("Simulation interval must be positive");
	}
}

int FixedStepClock::Advance(double deltaSeconds)
{
	if (deltaSeconds < 0.0)
	{
		throw std::invalid_argument("Frame time cannot be negative");
	}
	accumulated += std::min(deltaSeconds, 0.25);
	int steps = 0;
	while (accumulated + 0.000000001 >= interval)
	{
		accumulated -= interval;
		++steps;
	}
	return steps;
}

void FixedStepClock::Reset()
{
	accumulated = 0.0;
}

SnakeGame::SnakeGame(SnakeConfig config) : config(std::move(config)), random(config.seed)
{
	if (config.width < 2 || config.height < 2)
	{
		throw std::invalid_argument("Board must be at least 2 by 2");
	}
	if (config.initialBody.empty())
	{
		if (config.width < 4)
		{
			throw std::invalid_argument("Default snake needs a board at least four cells wide");
		}
		const int x = config.width / 2;
		const int z = config.height / 2;
		config.initialBody = {{x, z}, {x - 1, z}, {x - 2, z}};
	}
	for (size_t i = 0; i < config.initialBody.size(); ++i)
	{
		const Cell cell = config.initialBody[i];
		if (cell.x < 0 || cell.x >= config.width || cell.z < 0 || cell.z >= config.height ||
		    std::find(config.initialBody.begin(), config.initialBody.begin() + i, cell) !=
		        config.initialBody.begin() + i)
		{
			throw std::invalid_argument("Initial snake body is outside the board or overlaps itself");
		}
	}
	if (config.initialFood)
	{
		const Cell cell = *config.initialFood;
		if (cell.x < 0 || cell.x >= config.width || cell.z < 0 || cell.z >= config.height ||
		    std::find(config.initialBody.begin(), config.initialBody.end(), cell) != config.initialBody.end())
		{
			throw std::invalid_argument("Initial food must occupy a free board cell");
		}
	}
	Reset();
}

void SnakeGame::Reset()
{
	random.seed(config.seed);
	body.assign(config.initialBody.begin(), config.initialBody.end());
	direction = config.initialDirection;
	pendingDirection.reset();
	score = 0;
	status = GameStatus::running;
	food = config.initialFood;
	if (!food)
	{
		SpawnFood();
	}
}

void SnakeGame::QueueDirection(Direction direction)
{
	if (status != GameStatus::running || pendingDirection || Opposite(direction, direction))
	{
		return;
	}
	pendingDirection = direction;
}

void SnakeGame::Step()
{
	if (status != GameStatus::running)
	{
		return;
	}
	if (pendingDirection)
	{
		direction = *pendingDirection;
		pendingDirection.reset();
	}
	const Cell next = moved(body.front(), direction);
	if (next.x < 0 || next.x >= config.width || next.z < 0 || next.z >= config.height)
	{
		status = GameStatus::lost;
		return;
	}
	const bool eating = food && next == *food;
	auto occupiedEnd = body.end();
	if (!eating)
	{
		--occupiedEnd;
	}
	if (std::find(body.begin(), occupiedEnd, next) != occupiedEnd)
	{
		status = GameStatus::lost;
		return;
	}
	body.push_front(next);
	if (eating)
	{
		++score;
		SpawnFood();
	}
	else
	{
		body.pop_back();
	}
}

int SnakeGame::GetWidth() const
{
	return config.width;
}

int SnakeGame::GetHeight() const
{
	return config.height;
}

const Ursa::Deque<Cell>& SnakeGame::GetBody() const
{
	return body;
}

std::optional<Cell> SnakeGame::GetFood() const
{
	return food;
}

Direction SnakeGame::GetDirection() const
{
	return direction;
}

GameStatus SnakeGame::GetStatus() const
{
	return status;
}

int SnakeGame::GetScore() const
{
	return score;
}

void SnakeGame::SpawnFood()
{
	Ursa::Vector<Cell> freeCells;
	for (int z = 0; z < config.height; ++z)
	{
		for (int x = 0; x < config.width; ++x)
		{
			const Cell cell{x, z};
			if (std::find(body.begin(), body.end(), cell) == body.end())
			{
				freeCells.push_back(cell);
			}
		}
	}
	if (freeCells.empty())
	{
		food.reset();
		status = GameStatus::won;
		return;
	}
	std::uniform_int_distribution<size_t> distribution(0, freeCells.size() - 1);
	food = freeCells[distribution(random)];
}
} // namespace Ursa::Demo

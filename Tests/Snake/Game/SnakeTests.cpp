#include "SnakeGame.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace
{
using namespace Ursa::Demo;

void Expect(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

void TestFixedStep()
{
	FixedStepClock slow(0.2);
	FixedStepClock fast(0.2);
	int slowSteps = 0;
	int fastSteps = 0;
	for (int frame = 0; frame < 10; ++frame)
	{
		slowSteps += slow.Advance(0.1);
	}
	for (int frame = 0; frame < 20; ++frame)
	{
		fastSteps += fast.Advance(0.05);
	}
	Expect(slowSteps == 5 && fastSteps == 5, "Movement depends on render frame rate");
	slow.Reset();
	Expect(slow.Advance(0.1) == 0, "Clock reset did not clear partial time");
	Expect(fast.Advance(2.0) <= 2, "Long frame caused excessive catch-up steps");
}

void TestDirectionBuffer()
{
	SnakeConfig config;
	config.width = config.height = 8;
	config.initialFood = Cell{7, 7};
	SnakeGame game(config);
	game.QueueDirection(Direction::Left);
	game.QueueDirection(Direction::Up);
	game.QueueDirection(Direction::Down);
	game.Step();
	Expect(game.GetDirection() == Direction::Up && game.GetBody().front() == Cell{4, 3},
	       "Direction buffering or reverse protection failed");
	game.QueueDirection(Direction::Down);
	game.Step();
	Expect(game.GetDirection() == Direction::Up && game.GetBody().front() == Cell{4, 2},
	       "Direct reversal changed snake direction");
}

void TestFoodAndReset()
{
	SnakeConfig config;
	config.width = config.height = 4;
	config.initialBody = {{1, 1}, {0, 1}};
	config.initialFood = Cell{2, 1};
	SnakeGame game(config);
	game.Step();
	Expect(game.GetScore() == 1 && game.GetBody().size() == 3, "Eating did not grow snake and score");
	Expect(game.GetFood() &&
	           std::find(game.GetBody().begin(), game.GetBody().end(), *game.GetFood()) == game.GetBody().end(),
	       "Food appeared on the snake");
	game.Reset();
	Expect(game.GetScore() == 0 && game.GetBody().size() == 2 && game.GetFood() == config.initialFood,
	       "Restart did not restore the initial state");
}

void TestCollisionAndTail()
{
	SnakeConfig wall;
	wall.width = wall.height = 4;
	wall.initialBody = {{3, 1}, {2, 1}};
	wall.initialFood = Cell{0, 0};
	SnakeGame wallGame(wall);
	wallGame.Step();
	Expect(wallGame.GetStatus() == GameStatus::Lost && wallGame.GetBody().front() == Cell{3, 1},
	       "Wall collision did not stop the game");
	wallGame.Reset();
	Expect(wallGame.GetStatus() == GameStatus::Running, "Restart did not clear the loss state");

	SnakeConfig loop;
	loop.width = loop.height = 4;
	loop.initialBody = {{1, 1}, {1, 2}, {2, 2}, {2, 1}};
	loop.initialDirection = Direction::Up;
	loop.initialFood = Cell{0, 0};
	SnakeGame tailGame(loop);
	tailGame.QueueDirection(Direction::Right);
	tailGame.Step();
	Expect(tailGame.GetStatus() == GameStatus::Running && tailGame.GetBody().front() == Cell{2, 1},
	       "Moving into the vacated tail should be legal");

	loop.initialBody = {{1, 1}, {1, 2}, {2, 2}, {2, 1}, {3, 1}};
	SnakeGame bodyGame(loop);
	bodyGame.QueueDirection(Direction::Right);
	bodyGame.Step();
	Expect(bodyGame.GetStatus() == GameStatus::Lost, "Body collision was not detected");
}

void TestWin()
{
	SnakeConfig config;
	config.width = config.height = 2;
	config.initialBody = {{0, 0}, {0, 1}, {1, 1}};
	config.initialFood = Cell{1, 0};
	SnakeGame game(config);
	game.Step();
	Expect(game.GetStatus() == GameStatus::Won && !game.GetFood() && game.GetBody().size() == 4 && game.GetScore() == 1,
	       "Filling the board did not win cleanly");
	game.Step();
	Expect(game.GetBody().size() == 4, "Won game continued moving");
}
} // namespace

int main()
{
	try
	{
		TestFixedStep();
		TestDirectionBuffer();
		TestFoodAndReset();
		TestCollisionAndTail();
		TestWin();
		std::cout << "Snake rules tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Snake rules test failed: " << error.what() << '\n';
		return 1;
	}
}

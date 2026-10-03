#include <Core/Containers.hpp>
#include "SnakeController.hpp"
#include <array>

namespace Ursa::Demo
{
SnakeController::SnakeController(SnakeGame& game, bool stressWindow, bool smokeGameplay)
    : game(game), stressWindow(stressWindow), smokeGameplay(smokeGameplay)
{
}

Result<void> SnakeController::OnUpdate(const FrameContext& context)
{
	++frame;
	if (stressWindow && frame == 5)
	{
		context.window->Resize(800, 600);
	}
	if (stressWindow && frame == 10)
	{
		context.window->Minimize();
		context.window->Restore();
	}
	if (context.input.KeyDown(Key::escape))
	{
		context.window->RequestClose();
	}
	ReadInput(context);
	if (smokeGameplay)
	{
		return RunSmokeStep();
	}
	return {};
}

Result<void> SnakeController::OnFixedUpdate(const FrameContext& context)
{
	if (!smokeGameplay)
	{
		for (int step = 0, count = clock.Advance(context.fixedDeltaSeconds); step < count; ++step)
		{
			game.Step();
		}
	}
	return {};
}

void SnakeController::ReadInput(const FrameContext& context)
{
	const Ursa::Array<Key, 8> keys{Key::up, Key::w, Key::down, Key::s, Key::left, Key::a, Key::right, Key::d};
	const Ursa::Array<Direction, 8> directions{Direction::up,   Direction::up,   Direction::down,  Direction::down,
	                                           Direction::left, Direction::left, Direction::right, Direction::right};
	for (size_t i = 0; i < keys.size(); ++i)
	{
		const bool down = context.input.KeyDown(keys[i]);
		if (down && !previousKeys[i])
		{
			game.QueueDirection(directions[i]);
		}
		previousKeys[i] = down;
	}
	const bool space = context.input.KeyDown(Key::space);
	if (space && !previousSpace)
	{
		game.Reset();
		clock.Reset();
	}
	previousSpace = space;
}

Result<void> SnakeController::RunSmokeStep()
{
	if (frame == 1)
	{
		game.Step();
		if (game.GetScore() != 1)
		{
			return Result<void>::Failure("Smoke game did not eat initial food");
		}
	}
	else if (frame == 2 || frame == 3)
	{
		game.Step();
		if (frame == 3 && game.GetStatus() != GameStatus::lost)
		{
			return Result<void>::Failure("Smoke game did not reach game over");
		}
	}
	else if (frame == 4)
	{
		game.Reset();
		if (game.GetStatus() != GameStatus::running || game.GetScore() != 0)
		{
			return Result<void>::Failure("Smoke game did not restart");
		}
	}
	return {};
}
} // namespace Ursa::Demo

#include <Core/Types.hpp>
#include <Core/Containers.hpp>
#include "SnakeController.hpp"

namespace Ursa::Demo
{
SnakeController::SnakeController(SnakeGame& snakeGame, bool enableWindowStress, bool enableGameplaySmoke)
    : game(snakeGame), stressWindow(enableWindowStress), smokeGameplay(enableGameplaySmoke)
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
	if (context.input.KeyDown(Key::Escape))
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
	const Ursa::Array<Key, 8> keys{Key::Up, Key::W, Key::Down, Key::S, Key::Left, Key::A, Key::Right, Key::D};
	const Ursa::Array<Direction, 8> directions{Direction::Up,   Direction::Up,   Direction::Down,  Direction::Down,
	                                           Direction::Left, Direction::Left, Direction::Right, Direction::Right};
	for (Ursa::Size i = 0; i < keys.size(); ++i)
	{
		const bool down = context.input.KeyDown(keys[i]);
		if (down && !previousKeys[i])
		{
			game.QueueDirection(directions[i]);
		}
		previousKeys[i] = down;
	}
	const bool space = context.input.KeyDown(Key::Space);
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
		if (frame == 3 && game.GetStatus() != GameStatus::Lost)
		{
			return Result<void>::Failure("Smoke game did not reach game over");
		}
	}
	else if (frame == 4)
	{
		game.Reset();
		if (game.GetStatus() != GameStatus::Running || game.GetScore() != 0)
		{
			return Result<void>::Failure("Smoke game did not restart");
		}
	}
	return {};
}
} // namespace Ursa::Demo

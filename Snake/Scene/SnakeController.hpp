#pragma once
#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <Scene/Component.hpp>
#include <array>

namespace Ursa::Demo
{
class SnakeController final : public Component
{
public:
	SnakeController(SnakeGame& game, bool stressWindow, bool smokeGameplay);

	Result<void> OnUpdate(const FrameContext& context) override;

	Result<void> OnFixedUpdate(const FrameContext& context) override;

	bool StressWindow() const
	{
		return stressWindow;
	}

	bool SmokeGameplay() const
	{
		return smokeGameplay;
	}

	void SetStressWindow(bool value)
	{
		stressWindow = value;
	}

	void SetSmokeGameplay(bool value)
	{
		smokeGameplay = value;
	}

private:
	Result<void> RunSmokeStep();

	void ReadInput(const FrameContext& context);

	SnakeGame& game;
	FixedStepClock clock;
	Ursa::Array<bool, 8> previousKeys{};
	uint32_t frame = 0;
	bool stressWindow = false;
	bool smokeGameplay = false;
	bool previousSpace = false;
};
} // namespace Ursa::Demo

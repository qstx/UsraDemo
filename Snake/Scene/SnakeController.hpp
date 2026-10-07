#pragma once
#include <Core/Types.hpp>
#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <Scene/Component.hpp>

namespace Ursa::Demo
{
class SnakeController final : public Component
{
public:
	String SerializationType() const override
	{
		return "demo.SnakeController";
	}

	Result<void> Serialize(ArchiveWriter& writer) const override;

	Result<void> Deserialize(const ArchiveReader& reader, SerializationContext& context) override;

	SnakeController(SnakeGame& snakeGame, bool enableWindowStress, bool enableGameplaySmoke);

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
	Ursa::UInt32 frame = 0;
	bool stressWindow = false;
	bool smokeGameplay = false;
	bool previousSpace = false;
};
} // namespace Ursa::Demo

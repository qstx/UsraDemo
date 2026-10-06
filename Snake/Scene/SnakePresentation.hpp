#pragma once
#include <Core/Types.hpp>
#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <Scene/Scene.hpp>
#include <string>

namespace Ursa::Demo
{
class SnakePresentation final : public Component
{
public:
	SnakePresentation(SnakeGame& snakeGame, GameObject::Ptr rootObject, bool enableGameplaySmoke);

	Result<void> OnLateUpdate(const FrameContext& context) override;

	const Ursa::Vector<GameObject::Ptr>& Segments() const
	{
		return segments;
	}

	GameObject::Ptr FoodNode() const
	{
		return foodNode;
	}

	GameObject::Ptr RootNode() const
	{
		return root;
	}

	bool SmokeGameplay() const
	{
		return smokeGameplay;
	}

	void SetRootNode(GameObject::Ptr rootObject)
	{
		this->root = rootObject;
	}

	void SetSmokeGameplay(bool value)
	{
		smokeGameplay = value;
	}

private:
	Result<void> SyncBody();

	Result<void> SyncFood();

	void UpdateTitle(Window* window);

	LocalTransform CellTransform(Cell cell, Ursa::Float32 height, Ursa::Float32 size) const;

	SnakeGame& game;
	GameObject::Ptr root;
	Ursa::Vector<GameObject::Ptr> segments;
	GameObject::Ptr foodNode;
	String title;
	bool smokeGameplay = false;
};
} // namespace Ursa::Demo

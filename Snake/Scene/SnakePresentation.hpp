#pragma once
#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <Scene/Scene.hpp>
#include <string>
#include <vector>

namespace Ursa::Demo
{
class SnakePresentation final : public Component
{
public:
	SnakePresentation(SnakeGame& game, NodeId root, bool smokeGameplay);

	Result<void> OnLateUpdate(const FrameContext& context) override;

	const Ursa::Vector<NodeId>& Segments() const
	{
		return segments;
	}

	NodeId FoodNode() const
	{
		return foodNode;
	}

	NodeId RootNode() const
	{
		return root;
	}

	bool SmokeGameplay() const
	{
		return smokeGameplay;
	}

	void SetRootNode(NodeId root)
	{
		this->root = root;
	}

	void SetSmokeGameplay(bool value)
	{
		smokeGameplay = value;
	}

private:
	Result<void> SyncBody();

	Result<void> SyncFood();

	void UpdateTitle(Window* window);

	LocalTransform CellTransform(Cell cell, float height, float size) const;

	SnakeGame& game;
	NodeId root;
	Ursa::Vector<NodeId> segments;
	NodeId foodNode;
	String title;
	bool smokeGameplay = false;
};
} // namespace Ursa::Demo

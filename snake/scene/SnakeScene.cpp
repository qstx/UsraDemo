#include <core/Containers.hpp>
#include "SnakeScene.hpp"
#include "SnakeController.hpp"
#include "SnakePresentation.hpp"
#include <core/StringInterop.hpp>
#include <algorithm>
#include <cmath>

namespace Ursa::Demo
{
Ursa::Vector<BoxInstance> BuildSnakeScene(const SnakeGame& game)
{
	Ursa::Vector<BoxInstance> boxes;
	boxes.reserve(static_cast<size_t>(game.GetWidth() * game.GetHeight()) + game.GetBody().size() + 5);
	const float xOffset = static_cast<float>(game.GetWidth() - 1) * 0.5f;
	const float zOffset = static_cast<float>(game.GetHeight() - 1) * 0.5f;
	boxes.push_back(
	    {{{0, -0.42f, 0},
	      {static_cast<float>(game.GetWidth()) + 0.35f, 0.3f, static_cast<float>(game.GetHeight()) + 0.35f}},
	     {0.12f, 0.18f, 0.23f, 1}});
	for (int z = 0; z < game.GetHeight(); ++z)
	{
		for (int x = 0; x < game.GetWidth(); ++x)
		{
			const float shade = (x + z) % 2 == 0 ? 0.25f : 0.31f;
			boxes.push_back(
			    {{{static_cast<float>(x) - xOffset, -0.22f, static_cast<float>(z) - zOffset}, {0.96f, 0.08f, 0.96f}},
			     {shade * 0.55f, shade, shade * 0.75f, 1}});
		}
	}
	if (game.GetFood())
	{
		const auto food = *game.GetFood();
		boxes.push_back({{{static_cast<float>(food.x) - xOffset, 0.28f, static_cast<float>(food.z) - zOffset},
		                  {0.55f, 0.55f, 0.55f}},
		                 {1.0f, 0.45f, 0.12f, 1}});
	}
	size_t segment = 0;
	for (const auto cell : game.GetBody())
	{
		const Color color = segment++ == 0 ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
		boxes.push_back({{{static_cast<float>(cell.x) - xOffset, 0.34f, static_cast<float>(cell.z) - zOffset},
		                  {0.84f, 0.84f, 0.84f}},
		                 color});
	}
	return boxes;
}

Result<void> SetupSnakeScene(Scene& scene, SnakeGame& game, bool stressWindow, bool smokeGameplay)
{
	const auto root = scene.CreateNode();
	if (!root)
	{
		return Result<void>::Failure(root.error);
	}
	auto named = scene.SetNodeLabel(root.value, "Snake Game");
	if (!named)
	{
		return named;
	}
	const auto addBox = [&](NodeId parent, LocalTransform transform, Color color, const String& label) -> Result<void>
	{
		auto node = scene.CreateNode(parent);
		if (!node)
		{
			return Result<void>::Failure(node.error);
		}
		auto namedBox = scene.SetNodeLabel(node.value, label);
		if (!namedBox)
		{
			return namedBox;
		}
		auto placed = scene.SetLocalTransform(node.value, transform);
		if (!placed)
		{
			return placed;
		}
		auto component = scene.AddComponent<BoxRenderComponent>(node.value, color);
		if (!component)
		{
			return Result<void>::Failure(component.error);
		}
		return {};
	};

	const auto boardRoot = scene.CreateNode(root.value);
	if (!boardRoot)
	{
		return Result<void>::Failure(boardRoot.error);
	}
	named = scene.SetNodeLabel(boardRoot.value, "Board");
	if (!named)
	{
		return named;
	}
	LocalTransform base;
	base.position = {0, -0.42f, 0};
	base.scale = {static_cast<float>(game.GetWidth()) + 0.35f, 0.3f, static_cast<float>(game.GetHeight()) + 0.35f};
	auto added = addBox(boardRoot.value, base, {0.12f, 0.18f, 0.23f, 1}, "Board Base");
	if (!added)
	{
		return added;
	}
	const float xOffset = static_cast<float>(game.GetWidth() - 1) * 0.5f;
	const float zOffset = static_cast<float>(game.GetHeight() - 1) * 0.5f;
	for (int z = 0; z < game.GetHeight(); ++z)
	{
		for (int x = 0; x < game.GetWidth(); ++x)
		{
			LocalTransform tile;
			tile.position = {static_cast<float>(x) - xOffset, -0.22f, static_cast<float>(z) - zOffset};
			tile.scale = {0.96f, 0.08f, 0.96f};
			const float shade = (x + z) % 2 == 0 ? 0.25f : 0.31f;
			const String label =
			    "Tile (" + ToUrsaString(std::to_string(x)) + ", " + ToUrsaString(std::to_string(z)) + ")";
			added = addBox(boardRoot.value, tile, {shade * 0.55f, shade, shade * 0.75f, 1}, label);
			if (!added)
			{
				return added;
			}
		}
	}

	const auto cameraNode = scene.CreateNode(root.value);
	if (!cameraNode)
	{
		return Result<void>::Failure(cameraNode.error);
	}
	named = scene.SetNodeLabel(cameraNode.value, "Game Camera");
	if (!named)
	{
		return named;
	}
	const float board = static_cast<float>(std::max(game.GetWidth(), game.GetHeight()));
	const float cameraHeight = board * 0.9f;
	const float cameraDepth = board * 1.15f;
	LocalTransform cameraTransform;
	cameraTransform.position = {0, cameraHeight, cameraDepth};
	cameraTransform.rotation = Quat::FromAxisAngle({1, 0, 0}, -std::atan2(cameraHeight, cameraDepth));
	auto placed = scene.SetLocalTransform(cameraNode.value, cameraTransform);
	if (!placed)
	{
		return placed;
	}
	const auto camera = scene.AddComponent<CameraComponent>(cameraNode.value);
	if (!camera)
	{
		return Result<void>::Failure(camera.error);
	}
	auto selected = scene.SetActiveCamera(camera.value);
	if (!selected)
	{
		return selected;
	}

	const auto controller = scene.AddComponent<SnakeController>(root.value, game, stressWindow, smokeGameplay);
	if (!controller)
	{
		return Result<void>::Failure(controller.error);
	}
	const auto presentation = scene.AddComponent<SnakePresentation>(root.value, game, root.value, smokeGameplay);
	if (!presentation)
	{
		return Result<void>::Failure(presentation.error);
	}
	return {};
}
} // namespace Ursa::Demo

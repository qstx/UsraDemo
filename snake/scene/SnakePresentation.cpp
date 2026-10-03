#include <core/Containers.hpp>
#include "SnakePresentation.hpp"
#include <core/StringInterop.hpp>

namespace Ursa::Demo
{
SnakePresentation::SnakePresentation(SnakeGame& game, NodeId root, bool smokeGameplay)
    : game(game), root(root), smokeGameplay(smokeGameplay)
{
}

Result<void> SnakePresentation::OnLateUpdate(const FrameContext& context)
{
	auto body = SyncBody();
	if (!body)
	{
		return body;
	}
	auto food = SyncFood();
	if (!food)
	{
		return food;
	}
	UpdateTitle(context.window);
	if (smokeGameplay && context.window)
	{
		if (context.frameIndex == 3 && context.window->Title().find("Game Over") == Ursa::String::npos)
		{
			return Result<void>::Failure("Game-over feedback missing from window title");
		}
		if (context.frameIndex == 4 && context.window->Title().find("Score: 0") == Ursa::String::npos)
		{
			return Result<void>::Failure("Restart feedback missing from window title");
		}
	}
	return {};
}

Result<void> SnakePresentation::SyncBody()
{
	while (segments.size() > game.GetBody().size())
	{
		auto removed = GetScene().DestroyNode(segments.back());
		if (!removed)
		{
			return removed;
		}
		segments.pop_back();
	}
	while (segments.size() < game.GetBody().size())
	{
		auto node = GetScene().CreateNode(root);
		if (!node)
		{
			return Result<void>::Failure(node.error);
		}
		const Color color = segments.empty() ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
		auto box = GetScene().AddComponent<BoxRenderComponent>(node.value, color);
		if (!box)
		{
			return Result<void>::Failure(box.error);
		}
		segments.push_back(node.value);
	}
	size_t index = 0;
	for (Cell cell : game.GetBody())
	{
		const NodeId node = segments[index];
		auto transformed = GetScene().SetLocalTransform(node, CellTransform(cell, 0.34f, 0.84f));
		if (!transformed)
		{
			return transformed;
		}
		if (auto* box = GetScene().FindComponent<BoxRenderComponent>(node))
		{
			box->color = index == 0 ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
		}
		++index;
	}
	return {};
}

Result<void> SnakePresentation::SyncFood()
{
	if (!game.GetFood())
	{
		if (foodNode)
		{
			auto removed = GetScene().DestroyNode(foodNode);
			if (!removed)
			{
				return removed;
			}
			foodNode = {};
		}
		return {};
	}
	if (!foodNode)
	{
		auto node = GetScene().CreateNode(root);
		if (!node)
		{
			return Result<void>::Failure(node.error);
		}
		auto box = GetScene().AddComponent<BoxRenderComponent>(node.value, Color{1.0f, 0.45f, 0.12f, 1});
		if (!box)
		{
			return Result<void>::Failure(box.error);
		}
		foodNode = node.value;
	}
	return GetScene().SetLocalTransform(foodNode, CellTransform(*game.GetFood(), 0.28f, 0.55f));
}

void SnakePresentation::UpdateTitle(Window* window)
{
	if (!window)
	{
		return;
	}
	String newTitle = "Ursa Snake | Score: " + ToUrsaString(std::to_string(game.GetScore()));
	if (game.GetStatus() == GameStatus::lost)
	{
		newTitle += " | Game Over";
	}
	else if (game.GetStatus() == GameStatus::won)
	{
		newTitle += " | You Win";
	}
	newTitle += " | Arrows/WASD: turn | Space: restart | Esc: quit";
	if (title != newTitle)
	{
		window->SetTitle(newTitle);
		title = std::move(newTitle);
	}
}

LocalTransform SnakePresentation::CellTransform(Cell cell, float height, float size) const
{
	LocalTransform transform;
	transform.position = {static_cast<float>(cell.x) - static_cast<float>(game.GetWidth() - 1) * 0.5f, height,
	                      static_cast<float>(cell.z) - static_cast<float>(game.GetHeight() - 1) * 0.5f};
	transform.scale = {size, size, size};
	return transform;
}
} // namespace Ursa::Demo

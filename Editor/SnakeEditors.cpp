#include "SnakeEditors.hpp"

#include <SnakeController.hpp>
#include <SnakePresentation.hpp>
#include <functional>

namespace Ursa::Demo
{
namespace
{
Result<Ursa::Demo::SnakeGame*> SnakeGameService(Editor::ComponentInspectorContext& context)
{
	return context.loadContext ? context.loadContext->Service<Ursa::Demo::SnakeGame>()
	                           : Result<Ursa::Demo::SnakeGame*>::Failure("SnakeGame service is unavailable");
}
} // namespace

Result<void> RegisterSnakeEditors(Editor::ComponentEditors& editors)
{
	Editor::ComponentEditor drawController = [](Component& component, Editor::ComponentInspectorContext&,
	                                            Editor::InspectorUI& ui) -> Result<bool>
	{
		auto& controller = static_cast<SnakeController&>(component);
		bool stress = controller.StressWindow();
		bool smoke = controller.SmokeGameplay();
		auto editedStress = ui.Checkbox("Stress window", stress);
		auto editedSmoke = ui.Checkbox("Smoke gameplay", smoke);
		if (!editedStress || !editedSmoke)
		{
			return Result<bool>::Failure(!editedStress ? editedStress.error : editedSmoke.error);
		}
		if (editedStress.value)
		{
			controller.SetStressWindow(stress);
		}
		if (editedSmoke.value)
		{
			controller.SetSmokeGameplay(smoke);
		}
		return Result<bool>::Success(editedStress.value || editedSmoke.value);
	};
	auto controller = editors.RegisterType(
	    {"demo.SnakeController", "Snake Controller", "Gameplay",
	     [](Editor::ComponentInspectorContext& context) -> Result<Component*>
	     {
		     auto game = SnakeGameService(context);
		     if (!game)
		     {
			     return Result<Component*>::Failure(game.error);
		     }
		     auto added = context.object->AddComponent<SnakeController>(*game.value, false, false);
		     return added ? Result<Component*>::Success(added.value) : Result<Component*>::Failure(added.error);
	     },
	     [](const Editor::ComponentInspectorContext& context)
	     {
		     return context.loadContext && context.loadContext->Service<SnakeGame>()
		                ? Result<void>{}
		                : Result<void>::Failure("Snake Controller requires a SnakeGame service");
	     }},
	    drawController);
	if (!controller)
	{
		return controller;
	}

	Editor::ComponentEditor drawPresentation = [](Component& component, Editor::ComponentInspectorContext& context,
	                                              Editor::InspectorUI& ui) -> Result<bool>
	{
		auto& presentation = static_cast<SnakePresentation&>(component);
		bool changed = false;
		bool smoke = presentation.SmokeGameplay();
		auto editedSmoke = ui.Checkbox("Smoke gameplay", smoke);
		if (!editedSmoke)
		{
			return Result<bool>::Failure(editedSmoke.error);
		}
		if (editedSmoke.value)
		{
			presentation.SetSmokeGameplay(smoke);
			changed = true;
		}
		Vector<Editor::InspectorOption> options{{{}, "None"}};
		std::function<void(GameObject::Ptr)> collect = [&](GameObject::Ptr candidate)
		{
			auto name = candidate->Name();
			if (name)
			{
				options.push_back({candidate->Id(), name.value});
			}
			for (GameObject::Ptr child : candidate->Children())
			{
				collect(child);
			}
		};
		for (GameObject::Ptr root : context.roots)
		{
			collect(root);
		}
		ResourceId rootId = presentation.RootNode() ? presentation.RootNode()->Id() : ResourceId{};
		auto selectedRoot = ui.Select("Root node", options, rootId);
		if (!selectedRoot)
		{
			return Result<bool>::Failure(selectedRoot.error);
		}
		if (selectedRoot.value)
		{
			GameObject::Ptr target;
			std::function<void(GameObject::Ptr)> find = [&](GameObject::Ptr candidate)
			{
				if (candidate->Id() == rootId)
				{
					target = candidate;
				}
				for (GameObject::Ptr child : candidate->Children())
				{
					find(child);
				}
			};
			for (GameObject::Ptr root : context.roots)
			{
				find(root);
			}
			if (!target)
			{
				return Result<bool>::Failure("Selected Snake root GameObject is unavailable");
			}
			presentation.SetRootNode(target);
			changed = true;
		}
		return Result<bool>::Success(changed);
	};
	return editors.RegisterType(
	    {"demo.SnakePresentation", "Snake Presentation", "Gameplay",
	     [](Editor::ComponentInspectorContext& context) -> Result<Component*>
	     {
		     auto game = SnakeGameService(context);
		     if (!game)
		     {
			     return Result<Component*>::Failure(game.error);
		     }
		     auto added = context.object->AddComponent<SnakePresentation>(*game.value, context.object, false);
		     return added ? Result<Component*>::Success(added.value) : Result<Component*>::Failure(added.error);
	     },
	     [](const Editor::ComponentInspectorContext& context)
	     {
		     return context.loadContext && context.loadContext->Service<SnakeGame>()
		                ? Result<void>{}
		                : Result<void>::Failure("Snake Presentation requires a SnakeGame service");
	     }},
	    drawPresentation);
}
} // namespace Ursa::Demo

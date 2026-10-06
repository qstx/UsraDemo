#include "SnakeEditors.hpp"
#include <SnakeController.hpp>
#include <SnakePresentation.hpp>
#include <imgui.h>
#include <functional>

namespace Ursa::Demo
{
Result<void> RegisterSnakeEditors(Editor::ComponentEditors& editors)
{
	auto controller = editors.RegisterEditor("demo.SnakeController",
	                                         [](Component& component, Scene&, GameObject::Ptr)
	                                         {
		                                         auto& value = static_cast<SnakeController&>(component);
		                                         bool stress = value.StressWindow();
		                                         bool smoke = value.SmokeGameplay();
		                                         const bool editedStress = ImGui::Checkbox("Stress window", &stress);
		                                         const bool editedSmoke = ImGui::Checkbox("Smoke gameplay", &smoke);
		                                         if (editedStress)
		                                         {
			                                         value.SetStressWindow(stress);
		                                         }
		                                         if (editedSmoke)
		                                         {
			                                         value.SetSmokeGameplay(smoke);
		                                         }
		                                         return Result<bool>::Success(editedStress || editedSmoke);
	                                         });
	if (!controller)
	{
		return controller;
	}
	return editors.RegisterEditor(
	    "demo.SnakePresentation",
	    [](Component& component, Scene& scene, GameObject::Ptr)
	    {
		    auto& value = static_cast<SnakePresentation&>(component);
		    bool changed = false;
		    bool smoke = value.SmokeGameplay();
		    if (ImGui::Checkbox("Smoke gameplay", &smoke))
		    {
			    value.SetSmokeGameplay(smoke);
			    changed = true;
		    }
		    const auto currentLabel = scene.GameObjectName(value.RootNode());
		    if (ImGui::BeginCombo("Root node", currentLabel ? currentLabel.value.c_str() : "Select node"))
		    {
			    std::function<void(GameObject::Ptr)> offer = [&](GameObject::Ptr candidate)
			    {
				    const auto label = scene.GameObjectName(candidate);
				    const auto id = scene.PersistentId(candidate);
				    if (label && id)
				    {
					    ImGui::PushID(id.value.ToString().c_str());
					    if (ImGui::Selectable(label.value.c_str(), value.RootNode() == candidate))
					    {
						    value.SetRootNode(candidate);
						    changed = true;
					    }
					    ImGui::PopID();
				    }
				    const auto children = scene.ChildrenOf(candidate);
				    if (children)
				    {
					    for (GameObject::Ptr child : children.value)
					    {
						    offer(child);
					    }
				    }
			    };
			    for (GameObject::Ptr root : scene.RootGameObjects())
			    {
				    offer(root);
			    }
			    ImGui::EndCombo();
		    }
		    return Result<bool>::Success(changed);
	    });
}
} // namespace Ursa::Demo

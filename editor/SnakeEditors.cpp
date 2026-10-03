#include "SnakeEditors.hpp"
#include <SnakeController.hpp>
#include <SnakePresentation.hpp>
#include <imgui.h>
#include <functional>

namespace ursa::demo {
Result<void> registerSnakeEditors(editor::ComponentEditors &editors) {
    auto controller = editors.registerEditor("demo.SnakeController", [](Component &component, Scene &, NodeId) {
        auto &value = static_cast<SnakeController &>(component);
        bool stress = value.stressWindow();
        bool smoke = value.smokeGameplay();
        const bool editedStress = ImGui::Checkbox("Stress window", &stress);
        const bool editedSmoke = ImGui::Checkbox("Smoke gameplay", &smoke);
        if (editedStress)
            value.setStressWindow(stress);
        if (editedSmoke)
            value.setSmokeGameplay(smoke);
        return Result<bool>::success(editedStress || editedSmoke);
    });
    if (!controller)
        return controller;
    return editors.registerEditor("demo.SnakePresentation", [](Component &component, Scene &scene, NodeId) {
        auto &value = static_cast<SnakePresentation &>(component);
        bool changed = false;
        bool smoke = value.smokeGameplay();
        if (ImGui::Checkbox("Smoke gameplay", &smoke)) {
            value.setSmokeGameplay(smoke);
            changed = true;
        }
        const auto currentLabel = scene.nodeLabel(value.rootNode());
        if (ImGui::BeginCombo("Root node", currentLabel ? currentLabel.value.c_str() : "Select node")) {
            std::function<void(NodeId)> offer = [&](NodeId candidate) {
                const auto label = scene.nodeLabel(candidate);
                const auto id = scene.nodePersistentId(candidate);
                if (label && id) {
                    ImGui::PushID(id.value.toString().c_str());
                    if (ImGui::Selectable(label.value.c_str(), value.rootNode() == candidate)) {
                        value.setRootNode(candidate);
                        changed = true;
                    }
                    ImGui::PopID();
                }
                const auto children = scene.childrenOf(candidate);
                if (children)
                    for (NodeId child : children.value)
                        offer(child);
            };
            for (NodeId root : scene.rootNodes())
                offer(root);
            ImGui::EndCombo();
        }
        return Result<bool>::success(changed);
    });
}
} // namespace ursa::demo

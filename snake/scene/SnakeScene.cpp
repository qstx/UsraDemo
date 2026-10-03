#include <core/Containers.hpp>
#include "SnakeScene.hpp"
#include "SnakeController.hpp"
#include "SnakePresentation.hpp"
#include <core/StringInterop.hpp>
#include <algorithm>
#include <cmath>

namespace ursa::demo {
ursa::Vector<BoxInstance> buildSnakeScene(const SnakeGame &game) {
    ursa::Vector<BoxInstance> boxes;
    boxes.reserve(static_cast<size_t>(game.width() * game.height()) + game.body().size() + 5);
    const float xOffset = static_cast<float>(game.width() - 1) * 0.5f;
    const float zOffset = static_cast<float>(game.height() - 1) * 0.5f;
    boxes.push_back(
        {{{0, -0.42f, 0}, {static_cast<float>(game.width()) + 0.35f, 0.3f, static_cast<float>(game.height()) + 0.35f}},
         {0.12f, 0.18f, 0.23f, 1}});
    for (int z = 0; z < game.height(); ++z)
        for (int x = 0; x < game.width(); ++x) {
            const float shade = (x + z) % 2 == 0 ? 0.25f : 0.31f;
            boxes.push_back(
                {{{static_cast<float>(x) - xOffset, -0.22f, static_cast<float>(z) - zOffset}, {0.96f, 0.08f, 0.96f}},
                 {shade * 0.55f, shade, shade * 0.75f, 1}});
        }
    if (game.food()) {
        const auto food = *game.food();
        boxes.push_back({{{static_cast<float>(food.x) - xOffset, 0.28f, static_cast<float>(food.z) - zOffset},
                          {0.55f, 0.55f, 0.55f}},
                         {1.0f, 0.45f, 0.12f, 1}});
    }
    size_t segment = 0;
    for (const auto cell : game.body()) {
        const Color color = segment++ == 0 ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
        boxes.push_back({{{static_cast<float>(cell.x) - xOffset, 0.34f, static_cast<float>(cell.z) - zOffset},
                          {0.84f, 0.84f, 0.84f}},
                         color});
    }
    return boxes;
}

Result<void> setupSnakeScene(Scene &scene, SnakeGame &game, bool stressWindow, bool smokeGameplay) {
    const auto root = scene.createNode();
    if (!root)
        return Result<void>::failure(root.error);
    auto named = scene.setNodeLabel(root.value, "Snake Game");
    if (!named)
        return named;
    const auto addBox = [&](NodeId parent, LocalTransform transform, Color color,
                            const String &label) -> Result<void> {
        auto node = scene.createNode(parent);
        if (!node)
            return Result<void>::failure(node.error);
        auto namedBox = scene.setNodeLabel(node.value, label);
        if (!namedBox)
            return namedBox;
        auto placed = scene.setLocalTransform(node.value, transform);
        if (!placed)
            return placed;
        auto component = scene.addComponent<BoxRenderComponent>(node.value, color);
        if (!component)
            return Result<void>::failure(component.error);
        return {};
    };

    const auto boardRoot = scene.createNode(root.value);
    if (!boardRoot)
        return Result<void>::failure(boardRoot.error);
    named = scene.setNodeLabel(boardRoot.value, "Board");
    if (!named)
        return named;
    LocalTransform base;
    base.position = {0, -0.42f, 0};
    base.scale = {static_cast<float>(game.width()) + 0.35f, 0.3f, static_cast<float>(game.height()) + 0.35f};
    auto added = addBox(boardRoot.value, base, {0.12f, 0.18f, 0.23f, 1}, "Board Base");
    if (!added)
        return added;
    const float xOffset = static_cast<float>(game.width() - 1) * 0.5f;
    const float zOffset = static_cast<float>(game.height() - 1) * 0.5f;
    for (int z = 0; z < game.height(); ++z)
        for (int x = 0; x < game.width(); ++x) {
            LocalTransform tile;
            tile.position = {static_cast<float>(x) - xOffset, -0.22f, static_cast<float>(z) - zOffset};
            tile.scale = {0.96f, 0.08f, 0.96f};
            const float shade = (x + z) % 2 == 0 ? 0.25f : 0.31f;
            const String label = "Tile (" + toUrsaString(std::to_string(x)) + ", " +
                                 toUrsaString(std::to_string(z)) + ")";
            added = addBox(boardRoot.value, tile, {shade * 0.55f, shade, shade * 0.75f, 1}, label);
            if (!added)
                return added;
        }

    const auto cameraNode = scene.createNode(root.value);
    if (!cameraNode)
        return Result<void>::failure(cameraNode.error);
    named = scene.setNodeLabel(cameraNode.value, "Game Camera");
    if (!named)
        return named;
    const float board = static_cast<float>(std::max(game.width(), game.height()));
    const float cameraHeight = board * 0.9f;
    const float cameraDepth = board * 1.15f;
    LocalTransform cameraTransform;
    cameraTransform.position = {0, cameraHeight, cameraDepth};
    cameraTransform.rotation = Quat::fromAxisAngle({1, 0, 0}, -std::atan2(cameraHeight, cameraDepth));
    auto placed = scene.setLocalTransform(cameraNode.value, cameraTransform);
    if (!placed)
        return placed;
    const auto camera = scene.addComponent<CameraComponent>(cameraNode.value);
    if (!camera)
        return Result<void>::failure(camera.error);
    auto selected = scene.setActiveCamera(camera.value);
    if (!selected)
        return selected;

    const auto controller = scene.addComponent<SnakeController>(root.value, game, stressWindow, smokeGameplay);
    if (!controller)
        return Result<void>::failure(controller.error);
    const auto presentation = scene.addComponent<SnakePresentation>(root.value, game, root.value, smokeGameplay);
    if (!presentation)
        return Result<void>::failure(presentation.error);
    return {};
}
} // namespace ursa::demo

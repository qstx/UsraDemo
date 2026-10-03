#include <core/Containers.hpp>
#include "SnakePresentation.hpp"
#include <core/StringInterop.hpp>

namespace ursa::demo {
SnakePresentation::SnakePresentation(SnakeGame &game, NodeId root, bool smokeGameplay)
    : game_(game), root_(root), smokeGameplay_(smokeGameplay) {
}

Result<void> SnakePresentation::onLateUpdate(const FrameContext &context) {
    auto body = syncBody();
    if (!body)
        return body;
    auto food = syncFood();
    if (!food)
        return food;
    updateTitle(context.window);
    if (smokeGameplay_ && context.window) {
        if (context.frameIndex == 3 && context.window->title().find("Game Over") == ursa::String::npos)
            return Result<void>::failure("Game-over feedback missing from window title");
        if (context.frameIndex == 4 && context.window->title().find("Score: 0") == ursa::String::npos)
            return Result<void>::failure("Restart feedback missing from window title");
    }
    return {};
}

Result<void> SnakePresentation::syncBody() {
    while (segments_.size() > game_.body().size()) {
        auto removed = scene().destroyNode(segments_.back());
        if (!removed)
            return removed;
        segments_.pop_back();
    }
    while (segments_.size() < game_.body().size()) {
        auto node = scene().createNode(root_);
        if (!node)
            return Result<void>::failure(node.error);
        const Color color = segments_.empty() ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
        auto box = scene().addComponent<BoxRenderComponent>(node.value, color);
        if (!box)
            return Result<void>::failure(box.error);
        segments_.push_back(node.value);
    }
    size_t index = 0;
    for (Cell cell : game_.body()) {
        const NodeId node = segments_[index];
        auto transformed = scene().setLocalTransform(node, cellTransform(cell, 0.34f, 0.84f));
        if (!transformed)
            return transformed;
        if (auto *box = scene().findComponent<BoxRenderComponent>(node))
            box->color = index == 0 ? Color{0.35f, 0.95f, 0.5f, 1} : Color{0.12f, 0.65f, 0.42f, 1};
        ++index;
    }
    return {};
}

Result<void> SnakePresentation::syncFood() {
    if (!game_.food()) {
        if (foodNode_) {
            auto removed = scene().destroyNode(foodNode_);
            if (!removed)
                return removed;
            foodNode_ = {};
        }
        return {};
    }
    if (!foodNode_) {
        auto node = scene().createNode(root_);
        if (!node)
            return Result<void>::failure(node.error);
        auto box = scene().addComponent<BoxRenderComponent>(node.value, Color{1.0f, 0.45f, 0.12f, 1});
        if (!box)
            return Result<void>::failure(box.error);
        foodNode_ = node.value;
    }
    return scene().setLocalTransform(foodNode_, cellTransform(*game_.food(), 0.28f, 0.55f));
}

void SnakePresentation::updateTitle(Window *window) {
    if (!window)
        return;
    String title = "Ursa Snake | Score: " + toUrsaString(std::to_string(game_.score()));
    if (game_.status() == GameStatus::Lost)
        title += " | Game Over";
    else if (game_.status() == GameStatus::Won)
        title += " | You Win";
    title += " | Arrows/WASD: turn | Space: restart | Esc: quit";
    if (title != title_) {
        window->setTitle(title);
        title_ = std::move(title);
    }
}

LocalTransform SnakePresentation::cellTransform(Cell cell, float height, float size) const {
    LocalTransform transform;
    transform.position = {static_cast<float>(cell.x) - static_cast<float>(game_.width() - 1) * 0.5f, height,
                          static_cast<float>(cell.z) - static_cast<float>(game_.height() - 1) * 0.5f};
    transform.scale = {size, size, size};
    return transform;
}
} // namespace ursa::demo

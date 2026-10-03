#pragma once
#include <core/Containers.hpp>
#include "SnakeGame.hpp"
#include <scene/Scene.hpp>
#include <string>
#include <vector>

namespace ursa::demo {
class SnakePresentation final : public Component {
public:
    SnakePresentation(SnakeGame &game, NodeId root, bool smokeGameplay);

    Result<void> onLateUpdate(const FrameContext &context) override;

    const ursa::Vector<NodeId> &segments() const {
        return segments_;
    }

    NodeId foodNode() const {
        return foodNode_;
    }

    NodeId rootNode() const {
        return root_;
    }

    bool smokeGameplay() const {
        return smokeGameplay_;
    }

    void setRootNode(NodeId root) {
        root_ = root;
    }

    void setSmokeGameplay(bool value) {
        smokeGameplay_ = value;
    }

private:
    Result<void> syncBody();

    Result<void> syncFood();

    void updateTitle(Window *window);

    LocalTransform cellTransform(Cell cell, float height, float size) const;

    SnakeGame &game_;
    NodeId root_;
    ursa::Vector<NodeId> segments_;
    NodeId foodNode_;
    String title_;
    bool smokeGameplay_ = false;
};
} // namespace ursa::demo

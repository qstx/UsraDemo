#pragma once
#include <core/Containers.hpp>
#include "SnakeGame.hpp"
#include <renderer3d/Renderer3D.hpp>
#include <vector>
#include <scene/Scene.hpp>

namespace ursa::demo {
ursa::Vector<BoxInstance> buildSnakeScene(const SnakeGame &game);

Result<void> setupSnakeScene(Scene &scene, SnakeGame &game, bool stressWindow = false, bool smokeGameplay = false);
} // namespace ursa::demo

#pragma once
#include <core/Containers.hpp>
#include "SnakeGame.hpp"
#include <renderer3d/Renderer3D.hpp>
#include <vector>
#include <scene/Scene.hpp>

namespace Ursa::Demo
{
Ursa::Vector<BoxInstance> BuildSnakeScene(const SnakeGame& game);

Result<void> SetupSnakeScene(Scene& scene, SnakeGame& game, bool stressWindow = false, bool smokeGameplay = false);
} // namespace Ursa::Demo

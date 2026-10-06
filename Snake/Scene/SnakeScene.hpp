#pragma once
#include <Core/Containers.hpp>
#include "SnakeGame.hpp"
#include <Renderer3D/Renderer3D.hpp>
#include <Scene/Scene.hpp>

namespace Ursa::Demo
{
Ursa::Vector<BoxInstance> BuildSnakeScene(const SnakeGame& game);

Result<void> SetupSnakeScene(Scene& scene, SnakeGame& game, bool stressWindow = false, bool smokeGameplay = false);
} // namespace Ursa::Demo

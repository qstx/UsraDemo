#include "SnakeScene.hpp"
#include "SnakePresentation.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
void Expect(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

void ExpectNear(float actual, float expected)
{
	Expect(std::abs(actual - expected) < 0.0001f, "Scene position does not match grid position");
}
} // namespace

int main()
{
	try
	{
		Ursa::Demo::SnakeConfig config;
		config.width = config.height = 4;
		config.initialBody = {{1, 1}, {0, 1}};
		config.initialFood = Ursa::Demo::Cell{2, 1};
		Ursa::Demo::SnakeGame game(config);
		auto scene = Ursa::Demo::BuildSnakeScene(game);
		Expect(scene.size() == 1 + 16 + 1 + 2, "Scene does not contain board, food and snake");
		ExpectNear(scene[17].transform.position.x, 0.5f);
		ExpectNear(scene[18].transform.position.x, -0.5f);
		Expect(scene[18].color.green > scene[18].color.red, "Snake head has incorrect color");
		game.Step();
		scene = Ursa::Demo::BuildSnakeScene(game);
		Expect(scene.size() == 1 + 16 + 1 + 3, "Growing snake did not add a visible segment");
		ExpectNear(scene[18].transform.position.x, 0.5f);
		ExpectNear(scene[18].transform.position.z, -0.5f);
		game.Reset();
		Ursa::Scene graph;
		Expect(static_cast<bool>(Ursa::Demo::SetupSnakeScene(graph, game)), "Could not create snake scene graph");
		const auto roots = graph.RootNodes();
		Expect(roots.size() == 1, "Snake scene has the wrong root structure");
		auto* presentation = graph.FindComponent<Ursa::Demo::SnakePresentation>(roots[0]);
		Expect(presentation != nullptr, "Snake presentation component is missing");
		graph.BeginFrame();
		Expect(static_cast<bool>(graph.LateUpdate({})), "Initial scene synchronization failed");
		auto rendered = graph.CollectRenderData(1.0f);
		Expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 2,
		       "Scene graph did not render board, food and snake");
		game.Step();
		graph.BeginFrame();
		Expect(static_cast<bool>(graph.LateUpdate({})), "Growth synchronization failed");
		Expect(presentation->Segments().size() == 3, "Growth did not create a snake node");
		rendered = graph.CollectRenderData(1.0f);
		Expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 3,
		       "Growth was not visible in the same frame");
		game.Step();
		game.Step();
		Expect(game.GetStatus() == Ursa::Demo::GameStatus::lost, "Scripted game did not lose");
		game.Reset();
		graph.BeginFrame();
		Expect(static_cast<bool>(graph.LateUpdate({})), "Restart synchronization failed");
		Expect(presentation->Segments().size() == 2, "Restart did not remove excess snake nodes");
		rendered = graph.CollectRenderData(1.0f);
		Expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 2,
		       "Restart scene has the wrong visible node count");
		std::cout << "Snake scene tests passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Snake scene test failed: " << error.what() << '\n';
		return 1;
	}
}

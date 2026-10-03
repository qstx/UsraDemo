#include "SnakeScene.hpp"
#include "SnakePresentation.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void expect(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void expectNear(float actual, float expected) {
    expect(std::abs(actual - expected) < 0.0001f, "Scene position does not match grid position");
}
} // namespace

int main() {
    try {
        ursa::demo::SnakeConfig config;
        config.width = config.height = 4;
        config.initialBody = {{1, 1}, {0, 1}};
        config.initialFood = ursa::demo::Cell{2, 1};
        ursa::demo::SnakeGame game(config);
        auto scene = ursa::demo::buildSnakeScene(game);
        expect(scene.size() == 1 + 16 + 1 + 2, "Scene does not contain board, food and snake");
        expectNear(scene[17].transform.position.x, 0.5f);
        expectNear(scene[18].transform.position.x, -0.5f);
        expect(scene[18].color.green > scene[18].color.red, "Snake head has incorrect color");
        game.step();
        scene = ursa::demo::buildSnakeScene(game);
        expect(scene.size() == 1 + 16 + 1 + 3, "Growing snake did not add a visible segment");
        expectNear(scene[18].transform.position.x, 0.5f);
        expectNear(scene[18].transform.position.z, -0.5f);
        game.reset();
        ursa::Scene graph;
        expect(static_cast<bool>(ursa::demo::setupSnakeScene(graph, game)), "Could not create snake scene graph");
        const auto roots = graph.rootNodes();
        expect(roots.size() == 1, "Snake scene has the wrong root structure");
        auto *presentation = graph.findComponent<ursa::demo::SnakePresentation>(roots[0]);
        expect(presentation != nullptr, "Snake presentation component is missing");
        graph.beginFrame();
        expect(static_cast<bool>(graph.lateUpdate({})), "Initial scene synchronization failed");
        auto rendered = graph.collectRenderData(1.0f);
        expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 2,
               "Scene graph did not render board, food and snake");
        game.step();
        graph.beginFrame();
        expect(static_cast<bool>(graph.lateUpdate({})), "Growth synchronization failed");
        expect(presentation->segments().size() == 3, "Growth did not create a snake node");
        rendered = graph.collectRenderData(1.0f);
        expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 3,
               "Growth was not visible in the same frame");
        game.step();
        game.step();
        expect(game.status() == ursa::demo::GameStatus::Lost, "Scripted game did not lose");
        game.reset();
        graph.beginFrame();
        expect(static_cast<bool>(graph.lateUpdate({})), "Restart synchronization failed");
        expect(presentation->segments().size() == 2, "Restart did not remove excess snake nodes");
        rendered = graph.collectRenderData(1.0f);
        expect(static_cast<bool>(rendered) && rendered.value.boxes.size() == 1 + 16 + 1 + 2,
               "Restart scene has the wrong visible node count");
        std::cout << "Snake scene tests passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Snake scene test failed: " << error.what() << '\n';
        return 1;
    }
}

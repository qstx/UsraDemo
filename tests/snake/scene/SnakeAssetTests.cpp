#include <SnakeController.hpp>
#include <SnakeGame.hpp>
#include <SnakeSerialization.hpp>
#include <serialization/JsonResource.hpp>
#include <serialization/PrefabResource.hpp>
#include <iostream>

int main(int argc, char **argv) {
    const std::filesystem::path project =
        argc > 1 ? std::filesystem::u8path(argv[1]) : std::filesystem::current_path();
    const std::filesystem::path root = project / "assets";
    const auto scenePath = root / "snake" / "snake.ursascene";
    const auto prefabPath = root / "snake" / "board.ursaprefab";
    auto sceneDocument = ursa::readJsonResource(scenePath, ursa::ResourceKind::Scene);
    auto prefabDocument = ursa::readJsonResource(prefabPath, ursa::ResourceKind::Prefab);
    if (!sceneDocument || !prefabDocument || !sceneDocument.value.contains("instances") ||
        sceneDocument.value["instances"].size() != 1 ||
        sceneDocument.value["instances"][0]["prefab"] != prefabDocument.value["id"]) {
        std::cerr << "Snake scene does not reference the board Prefab\n";
        return 1;
    }

    ursa::ComponentRegistry registry;
    auto registered = ursa::registerBuiltinComponents(registry);
    if (registered)
        registered = ursa::demo::registerSnakeComponents(registry);
    auto resources = ursa::ResourceSession::open({root});
    if (!registered || !resources) {
        std::cerr << (registered ? resources.error.c_str() : registered.error.c_str()) << '\n';
        return 1;
    }
    ursa::demo::SnakeGame game;
    ursa::ComponentLoadContext context(*resources.value);
    context.provide(game);
    auto loaded = ursa::loadLinkedSceneResource(scenePath, registry, context);
    if (!loaded) {
        std::cerr << loaded.error.c_str() << '\n';
        return 1;
    }
    auto roots = loaded.value.scene->rootNodes();
    auto frame = loaded.value.scene->collectRenderData(16.0f / 9.0f);
    if (roots.size() != 1 || !loaded.value.scene->findComponent<ursa::demo::SnakeController>(roots.front()) ||
        !frame || frame.value.boxes.size() != 257) {
        std::cerr << "Snake scene did not restore its gameplay components, camera, and board\n";
        return 1;
    }
    return 0;
}

#include <SnakeGame.hpp>
#include <SnakeScene.hpp>
#include <SnakeSerialization.hpp>
#include <core/StringInterop.hpp>
#include <core/Log.hpp>
#include <serialization/JsonResource.hpp>
#include <serialization/PrefabResource.hpp>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>

namespace {
ursa::ResourceId stableId(uint8_t kind, uint32_t index) {
    ursa::ResourceId id;
    id.bytes[0] = 0x75;
    id.bytes[1] = 0x72;
    id.bytes[2] = 0x73;
    id.bytes[3] = 0x61;
    id.bytes[8] = kind;
    for (unsigned byte = 0; byte < 4; ++byte)
        id.bytes[15 - byte] = static_cast<uint8_t>(index >> (byte * 8));
    return id;
}

ursa::Result<void> assignStableIds(ursa::Scene &scene) {
    uint32_t nextNode = 1;
    uint32_t nextComponent = 1;
    std::function<ursa::Result<void>(ursa::NodeId)> visit = [&](ursa::NodeId node) -> ursa::Result<void> {
        auto assigned = scene.assignNodePersistentId(node, stableId(1, nextNode++));
        if (!assigned)
            return assigned;
        auto components = scene.componentsOf(node);
        if (!components)
            return ursa::Result<void>::failure(components.error);
        for (ursa::ComponentId component : components.value) {
            assigned = scene.assignComponentPersistentId(component, stableId(2, nextComponent++));
            if (!assigned)
                return assigned;
        }
        auto children = scene.childrenOf(node);
        if (!children)
            return ursa::Result<void>::failure(children.error);
        for (ursa::NodeId child : children.value) {
            assigned = visit(child);
            if (!assigned)
                return assigned;
        }
        return {};
    };
    for (ursa::NodeId root : scene.rootNodes()) {
        auto assigned = visit(root);
        if (!assigned)
            return assigned;
    }
    return {};
}

ursa::Result<void> bake(const std::filesystem::path &assetRoot, bool force) {
    const auto directory = assetRoot / "snake";
    const auto prefabPath = directory / "board.ursaprefab";
    const auto scenePath = directory / "snake.ursascene";
    if (!force && (std::filesystem::exists(prefabPath) || std::filesystem::exists(scenePath)))
        return ursa::Result<void>::failure("Snake assets already exist; pass --force to replace them");
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
        return ursa::Result<void>::failure("Cannot create Snake asset directory: " +
                                           ursa::toUrsaString(error.message()));

    ursa::demo::SnakeGame game;
    ursa::Scene scene;
    auto prepared = ursa::demo::setupSnakeScene(scene, game);
    if (!prepared)
        return prepared;
    auto assigned = assignStableIds(scene);
    if (!assigned)
        return assigned;
    auto roots = scene.rootNodes();
    if (roots.size() != 1)
        return ursa::Result<void>::failure("Snake scene must have one root");
    auto children = scene.childrenOf(roots.front());
    if (!children || children.value.empty())
        return ursa::Result<void>::failure("Snake scene has no board subtree");
    const ursa::NodeId board = children.value.front();
    auto parentId = scene.nodePersistentId(roots.front());
    if (!parentId)
        return ursa::Result<void>::failure(parentId.error);

    ursa::ComponentRegistry registry;
    auto registered = ursa::registerBuiltinComponents(registry);
    if (registered)
        registered = ursa::demo::registerSnakeComponents(registry);
    if (!registered)
        return registered;
    const auto prefabId = stableId(3, 1);
    const auto sceneId = stableId(3, 2);
    const auto instanceId = stableId(3, 3);
    auto saved = ursa::savePrefabResource(prefabPath, prefabId, scene, board, registry);
    if (!saved)
        return saved;
    auto removed = scene.destroyNode(board);
    if (!removed)
        return removed;
    auto encoded = ursa::encodeSceneDocument(scenePath, sceneId, scene, registry);
    if (!encoded)
        return ursa::Result<void>::failure(encoded.error);
    auto linked = ursa::addPrefabInstance(encoded.value, prefabId, instanceId, parentId.value);
    if (!linked)
        return linked;
    return ursa::writeJsonResource(scenePath, encoded.value);
}
} // namespace

int main(int argc, char **argv) {
    ursa::LogSession logging;
    if (argc < 2 || argc > 3 || (argc == 3 && std::string(argv[2]) != "--force")) {
        std::cerr << "Usage: UrsaSnakeAssetBake <project-directory> [--force]\n";
        return 2;
    }
    auto result = bake(std::filesystem::u8path(argv[1]) / "assets", argc == 3);
    if (!result) {
        URSA_LOG_ERROR("Demo", result.error);
        return 1;
    }
    URSA_LOG_INFO("Demo", "Baked Snake assets");
    return 0;
}

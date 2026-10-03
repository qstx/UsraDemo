#include <core/Containers.hpp>
#include <core/StringInterop.hpp>
#include "SnakeScene.hpp"
#include "SnakeSerialization.hpp"
#include "SnakeController.hpp"
#include <serialization/SceneResource.hpp>
#include <serialization/PrefabResource.hpp>
#include <iostream>
#include <stdexcept>

namespace {
template <class T> T take(ursa::Result<T> result) {
    if (!result)
        throw std::runtime_error(ursa::toStdString(result.error));
    return std::move(result.value);
}

void done(ursa::Result<void> result) {
    if (!result)
        throw std::runtime_error(ursa::toStdString(result.error));
}

void expect(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void loadSnake(const std::filesystem::path &file, ursa::ResourceSession &resources, ursa::demo::SnakeGame &game) {
    ursa::ComponentRegistry registry;
    if (!ursa::registerBuiltinComponents(registry) || !ursa::demo::registerSnakeComponents(registry))
        return;
    ursa::ComponentLoadContext context(resources);
    context.provide(game);
    auto scene = ursa::loadSceneResource(file, registry, context);
}

void moveTile(ursa::LinkedSceneResource &level, const std::filesystem::path &scenePath,
              ursa::ComponentRegistry &registry, ursa::ComponentLoadContext &context, ursa::ResourceId instanceId,
              ursa::ResourceId tileNodeId) {
    if (!ursa::setPrefabPropertyOverride(level, instanceId, tileNodeId, "/transform/position/0", 3.0f) ||
        !ursa::saveLinkedSceneResource(scenePath, level, registry))
        return;
    if (!ursa::reloadLinkedSceneResource(scenePath, registry, context, level))
        return;
    if (!ursa::clearPrefabPropertyOverride(level, instanceId, tileNodeId, "/transform/position/0") ||
        !ursa::saveLinkedSceneResource(scenePath, level, registry))
        return;
    ursa::reloadLinkedSceneResource(scenePath, registry, context, level);
}

struct CleanupCount {
    int value = 0;
};

class CleanupComponent final : public ursa::Component {
public:
    CleanupComponent(ursa::demo::SnakeGame &game, CleanupCount &count) : game_(game), count_(count) {
    }

    void onRemove() override {
        ++count_.value;
    }

private:
    ursa::demo::SnakeGame &game_;
    CleanupCount &count_;
};

void registerCleanup(ursa::ComponentRegistry &registry) {
    ursa::ComponentCodec codec;
    codec.type = "test.SnakeCleanup";
    codec.matches = [](const ursa::Component &component) {
        return dynamic_cast<const CleanupComponent *>(&component) != nullptr;
    };
    codec.save = [](const ursa::Component &, const ursa::ComponentSaveContext &) {
        return ursa::Result<nlohmann::json>::success(nlohmann::json::object());
    };
    codec.create = [](ursa::Scene &scene, ursa::NodeId node, const nlohmann::json &,
                      ursa::ComponentLoadContext &context) {
        auto game = context.service<ursa::demo::SnakeGame>();
        auto count = context.service<CleanupCount>();
        if (!game || !count)
            return ursa::Result<ursa::ComponentId>::failure(!game ? game.error : count.error);
        return scene.addComponent<CleanupComponent>(node, *game.value, *count.value);
    };
    done(registry.registerCodec(std::move(codec)));
}

void test(const std::filesystem::path &root) {
    ursa::demo::SnakeConfig config;
    config.width = config.height = 4;
    config.initialBody = {{1, 1}, {0, 1}};
    config.initialFood = ursa::demo::Cell{2, 1};
    ursa::demo::SnakeGame game(config);
    CleanupCount cleanup;
    ursa::ComponentRegistry registry;
    done(ursa::registerBuiltinComponents(registry));
    done(ursa::demo::registerSnakeComponents(registry));
    registerCleanup(registry);
    ursa::Scene source;
    done(ursa::demo::setupSnakeScene(source, game, false, false));
    take(source.addComponent<CleanupComponent>(source.rootNodes()[0], game, cleanup));
    const auto path = root / "snake.ursascene";
    done(ursa::saveSceneResource(path, ursa::ResourceId::generate(), source, registry));
    auto session = take(ursa::ResourceSession::open({root}));
    loadSnake(path, *session, game);
    ursa::ComponentLoadContext context(*session);
    context.provide(game);
    context.provide(cleanup);
    auto loaded = take(ursa::loadSceneResource(path, registry, context));
    auto *controller = loaded.scene->findComponent<ursa::demo::SnakeController>(loaded.scene->rootNodes()[0]);
    expect(controller && !controller->stressWindow() && !controller->smokeGameplay(),
           "Snake controller fields were not restored");
    loaded.scene->beginFrame();
    done(loaded.scene->update({}));
    ursa::FrameContext frame;
    frame.fixedDeltaSeconds = 0.2f;
    done(loaded.scene->fixedUpdate(frame));
    done(loaded.scene->lateUpdate({}));
    expect(game.score() == 1, "Restored Snake controller did not advance the bound game");
    const int before = cleanup.value;
    loaded.scene->clear();
    expect(cleanup.value == before + 1, "Restored component cleanup callback was not called once");
    ursa::ComponentLoadContext missingContext(*session);
    missingContext.provide(cleanup);
    auto missing = ursa::loadSceneResource(path, registry, missingContext);
    expect(!missing && missing.error.find("Missing runtime service") != ursa::String::npos,
           "Missing SnakeGame dependency was accepted");
    std::filesystem::remove(path);
}
} // namespace

int main() {
    try {
        const auto root = std::filesystem::temp_directory_path() /
                          ("UrsaSnakeSerializationTests-" + ursa::toStdString(ursa::ResourceId::generate().toString()));
        std::filesystem::create_directory(root);
        test(root);
        std::filesystem::remove(root);
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

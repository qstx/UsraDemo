#include "SnakeGame.hpp"
#include "SnakeScene.hpp"
#include "SnakeSerialization.hpp"
#include <application/Application.hpp>
#include <core/Log.hpp>
#include <serialization/SceneResource.hpp>
#include <serialization/PrefabResource.hpp>
#include <core/StringInterop.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
ursa::demo::SnakeConfig smokeConfig() {
    ursa::demo::SnakeConfig config;
    config.width = config.height = 4;
    config.initialBody = {{1, 1}, {0, 1}};
    config.initialFood = ursa::demo::Cell{2, 1};
    return config;
}

class SnakeApp final : public ursa::Application {
public:
    SnakeApp(std::filesystem::path projectDirectory, bool stressWindow, bool smokeGameplay, bool serialized)
        : game_(smokeGameplay ? smokeConfig() : ursa::demo::SnakeConfig{}),
          assetsRoot_(std::move(projectDirectory) / "assets"), stressWindow_(stressWindow),
          smokeGameplay_(smokeGameplay), serialized_(serialized) {
    }

private:
    ursa::Result<void> onStart() override {
        if (!smokeGameplay_ && !stressWindow_)
            return loadAuthoredScene();
        scene_ = std::make_shared<ursa::Scene>();
        auto created = ursa::demo::setupSnakeScene(*scene_, game_, stressWindow_, smokeGameplay_);
        if (!created || !serialized_)
            return created;
        ursa::ComponentRegistry registry;
        auto registered = ursa::registerBuiltinComponents(registry);
        if (registered)
            registered = ursa::demo::registerSnakeComponents(registry);
        if (!registered)
            return registered;
        const auto resourceRoot = std::filesystem::temp_directory_path() /
                                  ("UrsaSnakeScene-" + ursa::toStdString(ursa::ResourceId::generate().toString()));
        std::error_code filesystemError;
        std::filesystem::create_directory(resourceRoot, filesystemError);
        if (filesystemError)
            return ursa::Result<void>::failure("Cannot create Snake resource root: " +
                                               ursa::toUrsaString(filesystemError.message()));
        const auto path = resourceRoot / "snake.ursascene";
        auto saved = ursa::saveSceneResource(path, ursa::ResourceId::generate(), *scene_, registry);
        if (!saved)
            return saved;
        auto session = ursa::ResourceSession::open({resourceRoot});
        if (!session)
            return ursa::Result<void>::failure(session.error);
        ursa::ComponentLoadContext context(*session.value);
        context.provide(game_);
        auto loaded = ursa::loadSceneResource(path, registry, context);
        if (!loaded)
            return ursa::Result<void>::failure(loaded.error);
        auto candidate = std::move(loaded.value.scene);
        auto reloaded = ursa::reloadSceneResource(path, registry, context, candidate);
        if (!reloaded)
            return reloaded;
        scene_ = std::shared_ptr<ursa::Scene>(std::move(candidate));
        std::filesystem::remove(path, filesystemError);
        std::filesystem::remove(resourceRoot, filesystemError);
        return {};
    }

    ursa::Result<void> loadAuthoredScene() {
        ursa::ComponentRegistry registry;
        auto registered = ursa::registerBuiltinComponents(registry);
        if (registered)
            registered = ursa::demo::registerSnakeComponents(registry);
        if (!registered)
            return registered;
        auto session = ursa::ResourceSession::open(assetsRoot_, ursa::MountAccess::ReadOnly);
        if (!session)
            return ursa::Result<void>::failure(session.error);
        ursa::ComponentLoadContext context(*session.value);
        context.provide(game_);
        const auto path = assetsRoot_ / "snake" / "snake.ursascene";
        auto loaded = ursa::loadLinkedSceneResource(path, registry, context);
        if (!loaded)
            return ursa::Result<void>::failure(loaded.error);
        if (serialized_) {
            auto reloaded = ursa::reloadLinkedSceneResource(path, registry, context, loaded.value);
            if (!reloaded)
                return reloaded;
        }
        scene_ = std::shared_ptr<ursa::Scene>(std::move(loaded.value.scene));
        return {};
    }

    ursa::demo::SnakeGame game_;
    std::filesystem::path assetsRoot_;
    bool stressWindow_ = false;
    bool smokeGameplay_ = false;
    bool serialized_ = false;
};
} // namespace

int main(int argc, char **argv) {
    ursa::LogSession logging;
    URSA_LOG_INFO("Demo", "Starting UrsaSnake");
    try {
        bool visible = true;
        bool stressWindow = false;
        bool smokeGameplay = false;
        bool serialized = false;
        uint32_t frameLimit = 0;
        std::filesystem::path projectDirectory = std::filesystem::current_path();
        for (int i = 1; i < argc; ++i) {
            const ursa::String argument = argv[i];
            if (argument == "--hidden")
                visible = false;
            else if (argument == "--stress-window")
                stressWindow = true;
            else if (argument == "--smoke-gameplay")
                smokeGameplay = true;
            else if (argument == "--serialized")
                serialized = true;
            else if (argument == "--frames" && i + 1 < argc)
                frameLimit = static_cast<uint32_t>(std::stoul(argv[++i]));
            else if (argument == "--project" && i + 1 < argc)
                projectDirectory = std::filesystem::u8path(argv[++i]);
            else
                throw std::runtime_error(
                    "Usage: UrsaSnake [--project <project-directory>] [--hidden] [--frames N] [--stress-window] "
                    "[--smoke-gameplay] [--serialized]");
        }
        SnakeApp app(projectDirectory, stressWindow, smokeGameplay, serialized);
        const auto result = app.run(1280, 720, "Ursa Snake", visible, frameLimit);
        if (!result)
            throw std::runtime_error(ursa::toStdString(result.error));
        URSA_LOG_INFO("Demo", "UrsaSnake stopped");
        return 0;
    } catch (const std::exception &error) {
        URSA_LOG_CRITICAL("Demo", ursa::String("UrsaSnake: ") + error.what());
        return 1;
    }
}

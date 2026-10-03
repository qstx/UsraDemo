#include <application/EditorApplication.hpp>
#include <core/Log.hpp>
#include <core/StringInterop.hpp>
#include <assets/SceneDraft.hpp>
#include <SnakeSerialization.hpp>
#include "SnakeEditors.hpp"
#include <SnakeGame.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {
class DemoEditor final : public ursa::editor::EditorApplication {
public:
    DemoEditor(ursa::editor::EditorProjectSetup setup, ursa::String initialScene)
        : EditorApplication(std::move(setup)), initialScene_(std::move(initialScene)) {
    }

protected:
    ursa::Result<void> onStart() override {
        if (initialScene_.empty())
            return {};
        auto path = ursa::VirtualPath::parse(
            ursa::String(ursa::editor::projectResourceScheme) + ":" + initialScene_);
        if (!path)
            return ursa::Result<void>::failure(path.error);
        auto target = workspace()->writablePath(path.value);
        if (!target)
            return ursa::Result<void>::failure(target.error);
        auto opened = ursa::editor::openSceneDocument(path.value, ursa::ResourceKind::Scene,
                                                       project().components, documents()->session(),
                                                       project().editContextSetup);
        if (!opened)
            return ursa::Result<void>::failure(opened.error);
        return documents()->requestOpen(std::move(opened.value));
    }

private:
    ursa::String initialScene_;
};
} // namespace

int main(int argc, char **argv) {
    ursa::LogSession logging;
    URSA_LOG_INFO("Editor", "Starting UrsaDemoEditor");
    bool visible = true;
    uint32_t frames = 0;
    std::filesystem::path projectRoot;
    std::filesystem::path initialScene;
    for (int index = 1; index < argc; ++index) {
        const ursa::String argument = argv[index];
        if (argument == "--hidden")
            visible = false;
        else if (argument == "--frames" && index + 1 < argc)
            frames = static_cast<uint32_t>(std::strtoul(argv[++index], nullptr, 10));
        else if (argument == "--project" && index + 1 < argc)
            projectRoot = std::filesystem::u8path(argv[++index]);
        else if (argument == "--scene" && index + 1 < argc)
            initialScene = std::filesystem::u8path(argv[++index]);
        else {
            std::cerr << "Unknown Demo Editor option: " << argument.c_str() << '\n';
            return 2;
        }
    }

    if (projectRoot.empty()) {
        std::cerr << "Usage: UrsaDemoEditor --project <project-directory> [--scene <relative-scene-path>] "
                     "[--hidden] [--frames N]\n";
        return 2;
    }
    if (!initialScene.empty() && initialScene.is_absolute()) {
        std::cerr << "--scene must be relative to the project assets folder\n";
        return 2;
    }
    const auto scenePath = initialScene.empty() ? ursa::String{} : ursa::toUrsaString(initialScene.generic_string());
    DemoEditor editor([](ursa::editor::EditorProject &project) {
        auto registered = ursa::demo::registerSnakeComponents(project.components);
        if (registered)
            registered = ursa::demo::registerSnakeEditors(project.componentEditors);
        if (registered) {
            ursa::demo::SnakeConfig config;
            config.initialBody = {{1, 1}, {0, 1}};
            config.initialFood = ursa::demo::Cell{2, 1};
            auto game = std::make_shared<ursa::demo::SnakeGame>(config);
            project.editContextSetup = [game](ursa::ComponentLoadContext &context) {
                context.provide(*game);
                return ursa::Result<void>{};
            };
            project.playContextSetup = [config](ursa::ComponentLoadContext &context)
                -> ursa::Result<std::shared_ptr<void>> {
                auto playGame = std::make_shared<ursa::demo::SnakeGame>(config);
                context.provide(*playGame);
                return ursa::Result<std::shared_ptr<void>>::success(playGame);
            };
        }
        return registered;
    }, scenePath);
    auto opened = editor.openProject(projectRoot);
    if (!opened) {
        URSA_LOG_CRITICAL("Editor", opened.error);
        return 1;
    }
    auto result = editor.run(1280, 720, "Ursa Demo Editor", visible, frames);
    if (!result) {
        URSA_LOG_CRITICAL("Editor", result.error);
        return 1;
    }
    URSA_LOG_INFO("Editor", "UrsaDemoEditor stopped");
    return 0;
}

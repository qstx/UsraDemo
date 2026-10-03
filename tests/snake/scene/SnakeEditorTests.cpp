#include <assets/SceneDraft.hpp>
#include <SnakeController.hpp>
#include <SnakePresentation.hpp>
#include <SnakeSerialization.hpp>
#include <SnakeGame.hpp>
#include <SnakeEditors.hpp>
#include <core/StringInterop.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
using namespace ursa;

template <class T> T take(Result<T> value) {
    if (!value)
        throw std::runtime_error(toStdString(value.error));
    return std::move(value.value);
}

void done(Result<void> value) {
    if (!value)
        throw std::runtime_error(toStdString(value.error));
}

void expect(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
} // namespace

int main() {
    const auto root = std::filesystem::current_path() /
                      ("snake_editor_" + toStdString(ResourceId::generate().toString()));
    try {
        std::filesystem::create_directories(root / "assets");
        ComponentRegistry codecs;
        done(registerBuiltinComponents(codecs));
        done(demo::registerSnakeComponents(codecs));
        editor::ComponentEditors inspectors;
        done(demo::registerSnakeEditors(inspectors));
        expect(inspectors.find("demo.SnakeController") && inspectors.find("demo.SnakePresentation"),
               "Snake property editors were not registered");
        expect(!inspectors.find("other.Component") && !inspectors.registerEditor("demo.SnakeController", {}),
               "Unregistered or duplicate property editor handling changed");
        auto workspace = take(editor::ProjectWorkspace::open(root));
        auto documents = take(editor::EditorDocuments::create(*workspace));
        demo::SnakeGame editGame;
        const editor::EditContextSetup setup = [&editGame](ComponentLoadContext &context) {
            context.provide(editGame);
            return Result<void>{};
        };
        const auto path = take(VirtualPath::parse("prj:snake.ursascene"));
        auto document = take(editor::newSceneDocument(path, ResourceKind::Scene, codecs,
                                                      documents->session(), setup));
        auto draft = document->payload<editor::SceneDraft>();
        const NodeId node = draft->scene().rootNodes().front();
        auto controllerId = take(draft->scene().addComponent<demo::SnakeController>(node, editGame, false, false));
        auto presentationId = take(draft->scene().addComponent<demo::SnakePresentation>(node, editGame, node, false));
        auto *controller = static_cast<demo::SnakeController *>(draft->scene().component(controllerId));
        auto *presentation = static_cast<demo::SnakePresentation *>(draft->scene().component(presentationId));
        controller->setStressWindow(true);
        controller->setSmokeGameplay(true);
        presentation->setSmokeGameplay(true);
        document->markDirty();
        done(documents->requestOpen(std::move(document)));
        done(documents->saveCurrent());
        auto reopened = take(editor::openSceneDocument(path, ResourceKind::Scene, codecs,
                                                       documents->session(), setup));
        const auto loaded = reopened->payload<editor::SceneDraft>();
        const NodeId loadedNode = loaded->scene().rootNodes().front();
        const auto *loadedController = loaded->scene().findComponent<demo::SnakeController>(loadedNode);
        const auto *loadedPresentation = loaded->scene().findComponent<demo::SnakePresentation>(loadedNode);
        expect(loadedController && loadedController->stressWindow() && loadedController->smokeGameplay() &&
                   loadedPresentation && loadedPresentation->smokeGameplay() &&
                   loadedPresentation->rootNode() == loadedNode,
               "Snake component fields did not survive edit/save/reopen");
        std::filesystem::remove_all(root);
        std::cout << "Snake Editor component test passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::filesystem::remove_all(root);
        std::cerr << error.what() << '\n';
        return 1;
    }
}

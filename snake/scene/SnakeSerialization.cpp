#include "SnakeSerialization.hpp"
#include "SnakeController.hpp"
#include "SnakePresentation.hpp"
#include <core/StringInterop.hpp>

namespace ursa::demo {
namespace {
Result<bool> boolean(const nlohmann::json &data, const char *name) {
    if (!data.is_object() || !data.contains(name) || !data[name].is_boolean())
        return Result<bool>::failure(String("Invalid Snake component field ") + name);
    return Result<bool>::success(data[name].get<bool>());
}

Result<void> registerController(ComponentRegistry &registry) {
    ComponentCodec codec;
    codec.type = "demo.SnakeController";
    codec.version = 1;
    codec.matches = [](const Component &component) {
        return dynamic_cast<const SnakeController *>(&component) != nullptr;
    };
    codec.save = [](const Component &component, const ComponentSaveContext &) {
        const auto &controller = static_cast<const SnakeController &>(component);
        return Result<nlohmann::json>::success(
            {{"stressWindow", controller.stressWindow()}, {"smokeGameplay", controller.smokeGameplay()}});
    };
    codec.create = [](Scene &scene, NodeId node, const nlohmann::json &data, ComponentLoadContext &context) {
        auto stress = boolean(data, "stressWindow");
        auto smoke = boolean(data, "smokeGameplay");
        if (!stress || !smoke)
            return Result<ComponentId>::failure(!stress ? stress.error : smoke.error);
        auto game = context.service<SnakeGame>();
        if (!game)
            return Result<ComponentId>::failure(game.error);
        return scene.addComponent<SnakeController>(node, *game.value, stress.value, smoke.value);
    };
    return registry.registerCodec(std::move(codec));
}

Result<void> registerPresentation(ComponentRegistry &registry) {
    ComponentCodec codec;
    codec.type = "demo.SnakePresentation";
    codec.version = 1;
    codec.matches = [](const Component &component) {
        return dynamic_cast<const SnakePresentation *>(&component) != nullptr;
    };
    codec.save = [](const Component &component, const ComponentSaveContext &context) {
        const auto &presentation = static_cast<const SnakePresentation &>(component);
        auto root = context.nodeId(presentation.rootNode());
        if (!root)
            return Result<nlohmann::json>::failure(root.error);
        return Result<nlohmann::json>::success(
            {{"root", toStdString(root.value.toString())}, {"smokeGameplay", presentation.smokeGameplay()}});
    };
    codec.create = [](Scene &scene, NodeId node, const nlohmann::json &data, ComponentLoadContext &context) {
        auto smoke = boolean(data, "smokeGameplay");
        if (!smoke || !data.contains("root") || !data["root"].is_string())
            return Result<ComponentId>::failure(!smoke ? smoke.error : "Invalid Snake presentation root");
        auto game = context.service<SnakeGame>();
        if (!game)
            return Result<ComponentId>::failure(game.error);
        return scene.addComponent<SnakePresentation>(node, *game.value, node, smoke.value);
    };
    codec.bind = [](Scene &scene, ComponentId component, const nlohmann::json &data,
                    ComponentLoadContext &context) -> Result<void> {
        auto id = ResourceId::parse(toUrsaString(data["root"].get<std::string>()));
        if (!id)
            return Result<void>::failure(id.error);
        auto root = context.node(id.value);
        if (!root)
            return Result<void>::failure(root.error);
        static_cast<SnakePresentation *>(scene.component(component))->setRootNode(root.value);
        return {};
    };
    return registry.registerCodec(std::move(codec));
}
} // namespace

Result<void> registerSnakeComponents(ComponentRegistry &registry) {
    auto result = registerController(registry);
    if (result)
        result = registerPresentation(registry);
    return result;
}
} // namespace ursa::demo

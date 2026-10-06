#include "SnakeSerialization.hpp"
#include "SnakeController.hpp"
#include "SnakePresentation.hpp"
#include <Core/StringInterop.hpp>

namespace Ursa::Demo
{
namespace
{
Result<bool> Boolean(const nlohmann::json& data, const char* name)
{
	if (!data.is_object() || !data.contains(name) || !data[name].is_boolean())
	{
		return Result<bool>::Failure(String("Invalid Snake component field ") + name);
	}
	return Result<bool>::Success(data[name].get<bool>());
}

Result<void> RegisterController(ComponentRegistry& registry)
{
	ComponentCodec codec;
	codec.type = "demo.SnakeController";
	codec.version = 1;
	codec.matches = [](const Component& component)
	{ return dynamic_cast<const SnakeController*>(&component) != nullptr; };
	codec.save = [](const Component& component, const ComponentSaveContext&)
	{
		const auto& controller = static_cast<const SnakeController&>(component);
		return Result<nlohmann::json>::Success(
		    {{"stressWindow", controller.StressWindow()}, {"smokeGameplay", controller.SmokeGameplay()}});
	};
	codec.create = [](Scene& scene, GameObject::Ptr node, const nlohmann::json& data, ComponentLoadContext& context)
	{
		auto stress = Boolean(data, "stressWindow");
		auto smoke = Boolean(data, "smokeGameplay");
		if (!stress || !smoke)
		{
			return Result<ComponentId>::Failure(!stress ? stress.error : smoke.error);
		}
		auto game = context.Service<SnakeGame>();
		if (!game)
		{
			return Result<ComponentId>::Failure(game.error);
		}
		return scene.AddComponent<SnakeController>(node, *game.value, stress.value, smoke.value);
	};
	codec.createDetached = [](GameObject::Ptr node, const nlohmann::json& data, ComponentLoadContext& context)
	{
		auto stress = Boolean(data, "stressWindow");
		auto smoke = Boolean(data, "smokeGameplay");
		if (!stress || !smoke)
		{
			return Result<Component*>::Failure(!stress ? stress.error : smoke.error);
		}
		auto game = context.Service<SnakeGame>();
		if (!game)
		{
			return Result<Component*>::Failure(game.error);
		}
		auto added = node->AddComponent<SnakeController>(*game.value, stress.value, smoke.value);
		return added ? Result<Component*>::Success(added.value) : Result<Component*>::Failure(added.error);
	};
	return registry.RegisterCodec(std::move(codec));
}

Result<void> RegisterPresentation(ComponentRegistry& registry)
{
	ComponentCodec codec;
	codec.type = "demo.SnakePresentation";
	codec.version = 1;
	codec.matches = [](const Component& component)
	{ return dynamic_cast<const SnakePresentation*>(&component) != nullptr; };
	codec.save = [](const Component& component, const ComponentSaveContext& context)
	{
		const auto& presentation = static_cast<const SnakePresentation&>(component);
		auto root = context.GetGameObjectId(presentation.RootNode());
		if (!root)
		{
			return Result<nlohmann::json>::Failure(root.error);
		}
		return Result<nlohmann::json>::Success(
		    {{"root", ToStdString(root.value.ToString())}, {"smokeGameplay", presentation.SmokeGameplay()}});
	};
	codec.create = [](Scene& scene, GameObject::Ptr node, const nlohmann::json& data, ComponentLoadContext& context)
	{
		auto smoke = Boolean(data, "smokeGameplay");
		if (!smoke || !data.contains("root") || !data["root"].is_string())
		{
			return Result<ComponentId>::Failure(!smoke ? smoke.error : "Invalid Snake presentation root");
		}
		auto game = context.Service<SnakeGame>();
		if (!game)
		{
			return Result<ComponentId>::Failure(game.error);
		}
		return scene.AddComponent<SnakePresentation>(node, *game.value, node, smoke.value);
	};
	codec.createDetached = [](GameObject::Ptr node, const nlohmann::json& data, ComponentLoadContext& context)
	{
		auto smoke = Boolean(data, "smokeGameplay");
		if (!smoke || !data.contains("root") || !data["root"].is_string())
		{
			return Result<Component*>::Failure(!smoke ? smoke.error : "Invalid Snake presentation root");
		}
		auto game = context.Service<SnakeGame>();
		if (!game)
		{
			return Result<Component*>::Failure(game.error);
		}
		auto added = node->AddComponent<SnakePresentation>(*game.value, node, smoke.value);
		return added ? Result<Component*>::Success(added.value) : Result<Component*>::Failure(added.error);
	};
	codec.bind = [](Scene& scene, ComponentId component, const nlohmann::json& data,
	                ComponentLoadContext& context) -> Result<void>
	{
		auto id = ResourceId::Parse(ToUrsaString(data["root"].get<std::string>()));
		if (!id)
		{
			return Result<void>::Failure(id.error);
		}
		auto root = context.GameObjectById(id.value);
		if (!root)
		{
			return Result<void>::Failure(root.error);
		}
		static_cast<SnakePresentation*>(scene.GetComponent(component))->SetRootNode(root.value);
		return {};
	};
	codec.bindDetached = [](Component* component, const nlohmann::json& data, ComponentLoadContext& context)
	{
		auto id = ResourceId::Parse(ToUrsaString(data["root"].get<std::string>()));
		if (!id)
		{
			return Result<void>::Failure(id.error);
		}
		auto root = context.GameObjectById(id.value);
		if (!root)
		{
			return Result<void>::Failure(root.error);
		}
		auto* presentation = dynamic_cast<SnakePresentation*>(component);
		if (!presentation)
		{
			return Result<void>::Failure("Invalid SnakePresentation component");
		}
		presentation->SetRootNode(root.value);
		return Result<void>::Success();
	};
	return registry.RegisterCodec(std::move(codec));
}
} // namespace

Result<void> RegisterSnakeComponents(ComponentRegistry& registry)
{
	auto result = RegisterController(registry);
	if (result)
	{
		result = RegisterPresentation(registry);
	}
	return result;
}
} // namespace Ursa::Demo

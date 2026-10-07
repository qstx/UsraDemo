#include "SnakeSerialization.hpp"
#include "SnakeController.hpp"
#include "SnakePresentation.hpp"

namespace Ursa::Demo
{
Result<void> SnakeController::Serialize(ArchiveWriter& writer) const
{
	auto result = writer.Write("stressWindow", stressWindow);
	return result ? writer.Write("smokeGameplay", smokeGameplay) : result;
}

Result<void> SnakeController::Deserialize(const ArchiveReader& reader, SerializationContext&)
{
	auto stress = reader.Boolean("stressWindow");
	auto smoke = reader.Boolean("smokeGameplay");
	if (!stress || !smoke)
	{
		return Result<void>::Failure(!stress ? stress.error : smoke.error);
	}
	stressWindow = stress.value;
	smokeGameplay = smoke.value;
	return {};
}

Result<void> SnakePresentation::Serialize(ArchiveWriter& writer) const
{
	if (!root)
	{
		return Result<void>::Failure("Empty Snake presentation root");
	}
	auto result = writer.Write("root", root->Id().ToString());
	return result ? writer.Write("smokeGameplay", smokeGameplay) : result;
}

Result<void> SnakePresentation::Deserialize(const ArchiveReader& reader, SerializationContext& context)
{
	auto rootText = reader.Text("root");
	auto smoke = reader.Boolean("smokeGameplay");
	if (!rootText || !smoke)
	{
		return Result<void>::Failure(!rootText ? rootText.error : smoke.error);
	}
	auto id = ResourceId::Parse(rootText.value);
	if (!id)
	{
		return Result<void>::Failure(id.error);
	}
	context.Defer(
	    [this, &context, id = id.value]() -> Result<void>
	    {
		    auto object = context.Object<GameObject>(id);
		    if (!object)
		    {
			    return Result<void>::Failure(object.error);
		    }
		    root = object.value->shared_from_this();
		    return {};
	    });
	smokeGameplay = smoke.value;
	return {};
}

Result<void> RegisterSnakeComponents(ComponentRegistry& registry)
{
	auto result = registry.RegisterSerializable<SnakeController>(
	    "demo.SnakeController", 1,
	    [](const GameObject::Ptr& owner, ComponentLoadContext& context) -> Result<SnakeController*>
	    {
		    auto game = context.Service<SnakeGame>();
		    return game ? owner->AddComponent<SnakeController>(*game.value, false, false)
		                : Result<SnakeController*>::Failure(game.error);
	    });
	if (result)
	{
		result = registry.RegisterSerializable<SnakePresentation>(
		    "demo.SnakePresentation", 1,
		    [](const GameObject::Ptr& owner, ComponentLoadContext& context) -> Result<SnakePresentation*>
		    {
			    auto game = context.Service<SnakeGame>();
			    return game ? owner->AddComponent<SnakePresentation>(*game.value, owner, false)
			                : Result<SnakePresentation*>::Failure(game.error);
		    });
	}
	return result;
}
} // namespace Ursa::Demo

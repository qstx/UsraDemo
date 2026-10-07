#include <ResourceFixtures.hpp>
#include <Core/Containers.hpp>
#include <Core/StringInterop.hpp>
#include "SnakeScene.hpp"
#include "SnakeSerialization.hpp"
#include "SnakeController.hpp"
#include <Serialization/SceneResource.hpp>
#include <Serialization/PrefabResource.hpp>
#include <iostream>
#include <stdexcept>

namespace
{
template <class T> T Take(Ursa::Result<T> result)
{
	if (!result)
	{
		throw std::runtime_error(Ursa::ToStdString(result.error));
	}
	return std::move(result.value);
}

void done(Ursa::Result<void> result)
{
	if (!result)
	{
		throw std::runtime_error(Ursa::ToStdString(result.error));
	}
}

void Expect(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}

void LoadSnake(const std::filesystem::path& file, Ursa::ResourceSession& resources, Ursa::Demo::SnakeGame& game)
{
	Ursa::ComponentRegistry registry;
	if (!Ursa::RegisterBuiltinComponents(registry) || !Ursa::Demo::RegisterSnakeComponents(registry))
	{
		return;
	}
	Ursa::ComponentLoadContext context(resources);
	context.Provide(game);
	auto scene = Ursa::LoadSceneResource(file, registry, context);
}

void MoveTile(Ursa::LinkedSceneResource& level, const std::filesystem::path& scenePath,
              Ursa::ComponentRegistry& registry, Ursa::ComponentLoadContext& context, Ursa::ResourceId instanceId,
              Ursa::ResourceId tileNodeId)
{
	if (!Ursa::SetPrefabPropertyOverride(level, instanceId, tileNodeId, "/localTransform/position/0", 3.0f) ||
	    !Ursa::SaveLinkedSceneResource(scenePath, level, registry))
	{
		return;
	}
	if (!Ursa::ReloadLinkedSceneResource(scenePath, registry, context, level))
	{
		return;
	}
	if (!Ursa::ClearPrefabPropertyOverride(level, instanceId, tileNodeId, "/localTransform/position/0") ||
	    !Ursa::SaveLinkedSceneResource(scenePath, level, registry))
	{
		return;
	}
	Ursa::ReloadLinkedSceneResource(scenePath, registry, context, level);
}

struct CleanupCount
{
	int value = 0;
};

class CleanupComponent final : public Ursa::Component
{
public:
	CleanupComponent(Ursa::Demo::SnakeGame& snakeGame, CleanupCount& cleanupCount)
	    : game(snakeGame), count(cleanupCount)
	{
	}

	void OnRemove() override
	{
		++count.value;
	}

private:
	Ursa::Demo::SnakeGame& game;
	CleanupCount& count;
};

void RegisterCleanup(Ursa::ComponentRegistry& registry)
{
	Ursa::ComponentCodec codec;
	codec.type = "test.SnakeCleanup";
	codec.matches = [](const Ursa::Component& component)
	{ return dynamic_cast<const CleanupComponent*>(&component) != nullptr; };
	codec.save = [](const Ursa::Component&, const Ursa::ComponentSaveContext&)
	{ return Ursa::Result<nlohmann::json>::Success(nlohmann::json::object()); };
	codec.create =
	    [](Ursa::Scene& scene, Ursa::GameObject::Ptr node, const nlohmann::json&, Ursa::ComponentLoadContext& context)
	{
		auto game = context.Service<Ursa::Demo::SnakeGame>();
		auto count = context.Service<CleanupCount>();
		if (!game || !count)
		{
			return Ursa::Result<Ursa::ComponentId>::Failure(!game ? game.error : count.error);
		}
		return scene.AddComponent<CleanupComponent>(node, *game.value, *count.value);
	};
	done(registry.RegisterCodec(std::move(codec)));
}

void Test(const std::filesystem::path& root)
{
	Ursa::Demo::SnakeConfig config;
	config.width = config.height = 4;
	config.initialBody = {{1, 1}, {0, 1}};
	config.initialFood = Ursa::Demo::Cell{2, 1};
	Ursa::Demo::SnakeGame game(config);
	CleanupCount cleanup;
	Ursa::ComponentRegistry registry;
	done(Ursa::RegisterBuiltinComponents(registry));
	done(Ursa::Demo::RegisterSnakeComponents(registry));
	RegisterCleanup(registry);
	Ursa::Scene source;
	done(Ursa::Demo::SetupSnakeScene(source, game, false, false));
	Take(source.AddComponent<CleanupComponent>(source.RootGameObjects()[0], game, cleanup));
	const auto path = root / "Snake.ursascene";
	done(Ursa::SaveSceneResource(path, Ursa::ResourceId::Generate(), source, registry));
	done(Ursa::WriteResourceMetadataTree(root));
	auto session = Take(Ursa::ResourceSession::Open({root}));
	LoadSnake(path, *session, game);
	Ursa::ComponentLoadContext context(*session);
	context.Provide(game);
	context.Provide(cleanup);
	auto loaded = Take(Ursa::LoadSceneResource(path, registry, context));
	auto* controller = loaded.scene->FindComponent<Ursa::Demo::SnakeController>(loaded.scene->RootGameObjects()[0]);
	Expect(controller && !controller->StressWindow() && !controller->SmokeGameplay(),
	       "Snake controller fields were not restored");
	loaded.scene->BeginFrame();
	done(loaded.scene->Update({}));
	Ursa::FrameContext frame;
	frame.fixedDeltaSeconds = 0.2f;
	done(loaded.scene->FixedUpdate(frame));
	done(loaded.scene->LateUpdate({}));
	Expect(game.GetScore() == 1, "Restored Snake controller did not advance the bound game");
	const int before = cleanup.value;
	loaded.scene->Clear();
	Expect(cleanup.value == before + 1, "Restored component cleanup callback was not called once");
	Ursa::ComponentLoadContext missingContext(*session);
	missingContext.Provide(cleanup);
	auto missing = Ursa::LoadSceneResource(path, registry, missingContext);
	Expect(!missing && missing.error.find("Missing runtime service") != Ursa::String::npos,
	       "Missing SnakeGame dependency was accepted");
	std::filesystem::remove(path);
}
} // namespace

int main()
{
	try
	{
		const auto root = std::filesystem::temp_directory_path() /
		                  ("UrsaSnakeSerializationTests-" + Ursa::ToStdString(Ursa::ResourceId::Generate().ToString()));
		std::filesystem::create_directory(root);
		Test(root);
		std::filesystem::remove_all(root);
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return 1;
	}
}

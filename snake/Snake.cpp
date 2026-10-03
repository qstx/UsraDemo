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

namespace
{
Ursa::Demo::SnakeConfig SmokeConfig()
{
	Ursa::Demo::SnakeConfig config;
	config.width = config.height = 4;
	config.initialBody = {{1, 1}, {0, 1}};
	config.initialFood = Ursa::Demo::Cell{2, 1};
	return config;
}

class SnakeApp final : public Ursa::Application
{
public:
	SnakeApp(std::filesystem::path projectDirectory, bool stressWindow, bool smokeGameplay, bool serialized)
	    : game(smokeGameplay ? SmokeConfig() : Ursa::Demo::SnakeConfig{}),
	      assetsRoot(std::move(projectDirectory) / "assets"), stressWindow(stressWindow), smokeGameplay(smokeGameplay),
	      serialized(serialized)
	{
	}

private:
	Ursa::Result<void> OnStart() override
	{
		if (!smokeGameplay && !stressWindow)
		{
			return LoadAuthoredScene();
		}
		scene = std::make_shared<Ursa::Scene>();
		auto created = Ursa::Demo::SetupSnakeScene(*scene, game, stressWindow, smokeGameplay);
		if (!created || !serialized)
		{
			return created;
		}
		Ursa::ComponentRegistry registry;
		auto registered = Ursa::RegisterBuiltinComponents(registry);
		if (registered)
		{
			registered = Ursa::Demo::RegisterSnakeComponents(registry);
		}
		if (!registered)
		{
			return registered;
		}
		const auto resourceRoot = std::filesystem::temp_directory_path() /
		                          ("UrsaSnakeScene-" + Ursa::ToStdString(Ursa::ResourceId::Generate().ToString()));
		std::error_code filesystemError;
		std::filesystem::create_directory(resourceRoot, filesystemError);
		if (filesystemError)
		{
			return Ursa::Result<void>::Failure("Cannot create Snake resource root: " +
			                                   Ursa::ToUrsaString(filesystemError.message()));
		}
		const auto path = resourceRoot / "snake.ursascene";
		auto saved = Ursa::SaveSceneResource(path, Ursa::ResourceId::Generate(), *scene, registry);
		if (!saved)
		{
			return saved;
		}
		auto session = Ursa::ResourceSession::Open({resourceRoot});
		if (!session)
		{
			return Ursa::Result<void>::Failure(session.error);
		}
		Ursa::ComponentLoadContext context(*session.value);
		context.Provide(game);
		auto loaded = Ursa::LoadSceneResource(path, registry, context);
		if (!loaded)
		{
			return Ursa::Result<void>::Failure(loaded.error);
		}
		auto candidate = std::move(loaded.value.scene);
		auto reloaded = Ursa::ReloadSceneResource(path, registry, context, candidate);
		if (!reloaded)
		{
			return reloaded;
		}
		scene = std::shared_ptr<Ursa::Scene>(std::move(candidate));
		std::filesystem::remove(path, filesystemError);
		std::filesystem::remove(resourceRoot, filesystemError);
		return {};
	}

	Ursa::Result<void> LoadAuthoredScene()
	{
		Ursa::ComponentRegistry registry;
		auto registered = Ursa::RegisterBuiltinComponents(registry);
		if (registered)
		{
			registered = Ursa::Demo::RegisterSnakeComponents(registry);
		}
		if (!registered)
		{
			return registered;
		}
		auto session = Ursa::ResourceSession::Open(assetsRoot, Ursa::MountAccess::readOnly);
		if (!session)
		{
			return Ursa::Result<void>::Failure(session.error);
		}
		Ursa::ComponentLoadContext context(*session.value);
		context.Provide(game);
		const auto path = assetsRoot / "snake" / "snake.ursascene";
		auto loaded = Ursa::LoadLinkedSceneResource(path, registry, context);
		if (!loaded)
		{
			return Ursa::Result<void>::Failure(loaded.error);
		}
		if (serialized)
		{
			auto reloaded = Ursa::ReloadLinkedSceneResource(path, registry, context, loaded.value);
			if (!reloaded)
			{
				return reloaded;
			}
		}
		scene = std::shared_ptr<Ursa::Scene>(std::move(loaded.value.scene));
		return {};
	}

	Ursa::Demo::SnakeGame game;
	std::filesystem::path assetsRoot;
	bool stressWindow = false;
	bool smokeGameplay = false;
	bool serialized = false;
};
} // namespace

int main(int argc, char** argv)
{
	Ursa::LogSession logging;
	URSA_LOG_INFO("Demo", "Starting UrsaSnake");
	try
	{
		bool visible = true;
		bool stressWindow = false;
		bool smokeGameplay = false;
		bool serialized = false;
		uint32_t frameLimit = 0;
		std::filesystem::path projectDirectory = std::filesystem::current_path();
		for (int i = 1; i < argc; ++i)
		{
			const Ursa::String argument = argv[i];
			if (argument == "--hidden")
			{
				visible = false;
			}
			else if (argument == "--stress-window")
			{
				stressWindow = true;
			}
			else if (argument == "--smoke-gameplay")
			{
				smokeGameplay = true;
			}
			else if (argument == "--serialized")
			{
				serialized = true;
			}
			else if (argument == "--frames" && i + 1 < argc)
			{
				frameLimit = static_cast<uint32_t>(std::stoul(argv[++i]));
			}
			else if (argument == "--project" && i + 1 < argc)
			{
				projectDirectory = std::filesystem::u8path(argv[++i]);
			}
			else
			{
				throw std::runtime_error(
				    "Usage: UrsaSnake [--project <project-directory>] [--hidden] [--frames N] [--stress-window] "
				    "[--smoke-gameplay] [--serialized]");
			}
		}
		SnakeApp app(projectDirectory, stressWindow, smokeGameplay, serialized);
		const auto result = app.Run(1280, 720, "Ursa Snake", visible, frameLimit);
		if (!result)
		{
			throw std::runtime_error(Ursa::ToStdString(result.error));
		}
		URSA_LOG_INFO("Demo", "UrsaSnake stopped");
		return 0;
	}
	catch (const std::exception& error)
	{
		URSA_LOG_CRITICAL("Demo", Ursa::String("UrsaSnake: ") + error.what());
		return 1;
	}
}

#include <Core/Types.hpp>
#include "SnakeGame.hpp"
#include "SnakeScene.hpp"
#include "SnakeSerialization.hpp"
#include <Application/Application.hpp>
#include <Core/Log.hpp>
#include <Serialization/SceneResource.hpp>
#include <Serialization/PrefabResource.hpp>
#include <Core/StringInterop.hpp>
#include <filesystem>
#include <Vfs/AssetRoots.hpp>
#include <Vfs/RuntimeMountProfile.hpp>
#include <Vfs/PakArchive.hpp>
#include <Vfs/MemoryFileProvider.hpp>
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
	SnakeApp(bool enableWindowStress, bool enableGameplaySmoke, bool enableSerializedReload,
	         std::shared_ptr<Ursa::VirtualFileSystem> selectedFiles, Ursa::VirtualPath selectedScene)
	    : game(enableGameplaySmoke ? SmokeConfig() : Ursa::Demo::SnakeConfig{}), files(std::move(selectedFiles)),
	      initialScene(std::move(selectedScene)), stressWindow(enableWindowStress), smokeGameplay(enableGameplaySmoke),
	      serialized(enableSerializedReload)
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

		Ursa::VirtualFileSystem scratch;
		auto provider = std::make_shared<Ursa::MemoryFileProvider>();
		auto mounted = scratch.MountProvider(Ursa::projectScheme, provider, Ursa::MountAccess::ReadWrite);
		if (!mounted)
		{
			return mounted;
		}
		auto path = Ursa::VirtualPath::Parse("prj:Snake.ursascene").value;
		auto saved = Ursa::SaveSceneResource(scratch, path, Ursa::ResourceId::Generate(), *scene, registry);
		if (!saved)
		{
			return saved;
		}
		auto session = Ursa::ResourceSession::Open(scratch);
		if (!session)
		{
			return Ursa::Result<void>::Failure(session.error);
		}
		Ursa::ComponentLoadContext context(*session.value);
		context.Provide(game);
		auto loaded = Ursa::LoadSceneResource(scratch, path, registry, context);
		if (!loaded)
		{
			return Ursa::Result<void>::Failure(loaded.error);
		}
		auto candidate = std::move(loaded.value.scene);
		auto reloaded = Ursa::ReloadSceneResource(scratch, path, registry, context, candidate);
		if (!reloaded)
		{
			return reloaded;
		}
		scene = std::shared_ptr<Ursa::Scene>(std::move(candidate));
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
		auto session = Ursa::ResourceSession::Open(*files);
		if (!session)
		{
			return Ursa::Result<void>::Failure(session.error);
		}
		Ursa::ComponentLoadContext context(*session.value);
		context.Provide(game);
		const auto path = initialScene;
		auto loaded = Ursa::LoadLinkedSceneResource(*files, path, registry, context);
		if (!loaded)
		{
			return Ursa::Result<void>::Failure(loaded.error);
		}
		if (serialized)
		{
			auto reloaded = Ursa::ReloadLinkedSceneResource(*files, path, registry, context, loaded.value);
			if (!reloaded)
			{
				return reloaded;
			}
		}
		scene = std::shared_ptr<Ursa::Scene>(std::move(loaded.value.scene));
		return {};
	}

	Ursa::Demo::SnakeGame game;
	std::shared_ptr<Ursa::VirtualFileSystem> files;
	Ursa::VirtualPath initialScene;
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
		Ursa::UInt32 frameLimit = 0;
		std::filesystem::path projectRoot = std::filesystem::current_path();
		std::filesystem::path package;
		Ursa::String sceneName = "Snake/Snake.ursascene";
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
			else if (argument == "--project" && i + 1 < argc)
			{
				projectRoot = std::filesystem::u8path(argv[++i]);
			}
			else if (argument == "--pak" && i + 1 < argc)
			{
				package = std::filesystem::u8path(argv[++i]);
			}
			else if (argument == "--scene" && i + 1 < argc)
			{
				sceneName = argv[++i];
			}
			else if (argument == "--frames" && i + 1 < argc)
			{
				frameLimit = static_cast<Ursa::UInt32>(std::stoul(argv[++i]));
			}
			else
			{
				throw std::runtime_error("Usage: UrsaSnake [--hidden] [--frames N] [--stress-window] "
				                         "[--smoke-gameplay] [--serialized]");
			}
		}

		Ursa::RuntimeMountProfileConfig profile;
		profile.applicationId = "UrsaSnake";
		profile.mode = package.empty() ? Ursa::RuntimeMountMode::LooseRuntime : Ursa::RuntimeMountMode::PackagedRuntime;
		if (package.empty())
		{
			auto roots = Ursa::ResolveEngineAssetRoots();
			if (!roots)
			{
				throw std::runtime_error(Ursa::ToStdString(roots.error));
			}
			profile.engineDirectories = std::move(roots.value);
			profile.projectAssetsDirectory = projectRoot / "Assets";
		}
		else
		{
			auto archive = Ursa::PakArchive::Open(package);
			if (!archive)
			{
				throw std::runtime_error(Ursa::ToStdString(archive.error));
			}
			for (const Ursa::String scheme : {Ursa::String(Ursa::engineScheme), Ursa::String(Ursa::projectScheme)})
			{
				auto view = archive.value->View(scheme);
				if (!view)
				{
					throw std::runtime_error(Ursa::ToStdString(view.error));
				}
				profile.packagedProviders.push_back({scheme, view.value});
			}
		}
		auto mounted = Ursa::OpenRuntimeFileSystem(profile);
		if (!mounted)
		{
			throw std::runtime_error(Ursa::ToStdString(mounted.error));
		}
		auto files = std::make_shared<Ursa::VirtualFileSystem>(std::move(mounted.value));
		Ursa::ConfigureEngineFileSystem(files);
		auto path = Ursa::VirtualPath::Parse("prj:" + sceneName);
		if (!path)
		{
			throw std::runtime_error(Ursa::ToStdString(path.error));
		}
		SnakeApp app(stressWindow, smokeGameplay, serialized, files, path.value);
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

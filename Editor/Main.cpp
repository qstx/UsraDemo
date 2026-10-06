#include <Core/Types.hpp>
#include <Application/EditorApplication.hpp>
#include <Core/Log.hpp>
#include <Core/StringInterop.hpp>
#include <Assets/SceneDraft.hpp>
#include <SnakeSerialization.hpp>
#include "SnakeEditors.hpp"
#include <SnakeGame.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{
class DemoEditor final : public Ursa::Editor::EditorApplication
{
public:
	DemoEditor(Ursa::Editor::EditorProjectSetup setup, Ursa::String scenePath)
	    : EditorApplication(std::move(setup)), initialScene(std::move(scenePath))
	{
	}

protected:
	Ursa::Result<void> OnStart() override
	{
		if (initialScene.empty())
		{
			return {};
		}
		auto path = Ursa::VirtualPath::Parse(Ursa::String(Ursa::Editor::projectResourceScheme) + ":" + initialScene);
		if (!path)
		{
			return Ursa::Result<void>::Failure(path.error);
		}
		auto target = Workspace()->WritablePath(path.value);
		if (!target)
		{
			return Ursa::Result<void>::Failure(target.error);
		}
		auto opened = Ursa::Editor::OpenSceneDocument(path.value, Ursa::ResourceKind::Scene, Project().components,
		                                              Documents()->Session(), Project().editContextSetup);
		if (!opened)
		{
			return Ursa::Result<void>::Failure(opened.error);
		}
		return Documents()->RequestOpen(std::move(opened.value));
	}

private:
	Ursa::String initialScene;
};
} // namespace

int main(int argc, char** argv)
{
	Ursa::LogSession logging;
	URSA_LOG_INFO("Editor", "Starting UrsaDemoEditor");
	bool visible = true;
	Ursa::UInt32 frames = 0;
	std::filesystem::path projectRoot;
	std::filesystem::path initialScene;
	for (int index = 1; index < argc; ++index)
	{
		const Ursa::String argument = argv[index];
		if (argument == "--hidden")
		{
			visible = false;
		}
		else if (argument == "--frames" && index + 1 < argc)
		{
			frames = static_cast<Ursa::UInt32>(std::strtoul(argv[++index], nullptr, 10));
		}
		else if (argument == "--project" && index + 1 < argc)
		{
			projectRoot = std::filesystem::u8path(argv[++index]);
		}
		else if (argument == "--scene" && index + 1 < argc)
		{
			initialScene = std::filesystem::u8path(argv[++index]);
		}
		else
		{
			std::cerr << "Unknown Demo Editor option: " << argument.c_str() << '\n';
			return 2;
		}
	}

	if (projectRoot.empty())
	{
		std::cerr << "Usage: UrsaDemoEditor --project <project-directory> [--scene <relative-scene-path>] "
		             "[--hidden] [--frames N]\n";
		return 2;
	}
	if (!initialScene.empty() && initialScene.is_absolute())
	{
		std::cerr << "--scene must be relative to the project assets folder\n";
		return 2;
	}
	const auto scenePath = initialScene.empty() ? Ursa::String{} : Ursa::ToUrsaString(initialScene.generic_string());
	DemoEditor editor(
	    [](Ursa::Editor::EditorProject& project)
	    {
		    auto registered = Ursa::Demo::RegisterSnakeComponents(project.components);
		    if (registered)
		    {
			    registered = Ursa::Demo::RegisterSnakeEditors(project.componentEditors);
		    }
		    if (registered)
		    {
			    Ursa::Demo::SnakeConfig config;
			    config.initialBody = {{1, 1}, {0, 1}};
			    config.initialFood = Ursa::Demo::Cell{2, 1};
			    auto game = std::make_shared<Ursa::Demo::SnakeGame>(config);
			    project.editContextSetup = [game](Ursa::ComponentLoadContext& context)
			    {
				    context.Provide(*game);
				    return Ursa::Result<void>{};
			    };
			    project.playContextSetup =
			        [config](Ursa::ComponentLoadContext& context) -> Ursa::Result<std::shared_ptr<void>>
			    {
				    auto playGame = std::make_shared<Ursa::Demo::SnakeGame>(config);
				    context.Provide(*playGame);
				    return Ursa::Result<std::shared_ptr<void>>::Success(playGame);
			    };
		    }
		    return registered;
	    },
	    scenePath);
	auto opened = editor.OpenProject(projectRoot);
	if (!opened)
	{
		URSA_LOG_CRITICAL("Editor", opened.error);
		return 1;
	}
	auto result = editor.Run(1280, 720, "Ursa Demo Editor", visible, frames);
	if (!result)
	{
		URSA_LOG_CRITICAL("Editor", result.error);
		return 1;
	}
	URSA_LOG_INFO("Editor", "UrsaDemoEditor stopped");
	return 0;
}

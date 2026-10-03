#include <Assets/SceneDraft.hpp>
#include <SnakeController.hpp>
#include <SnakePresentation.hpp>
#include <SnakeSerialization.hpp>
#include <SnakeGame.hpp>
#include <SnakeEditors.hpp>
#include <Core/StringInterop.hpp>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
using namespace Ursa;

template <class T> T Take(Result<T> value)
{
	if (!value)
	{
		throw std::runtime_error(ToStdString(value.error));
	}
	return std::move(value.value);
}

void done(Result<void> value)
{
	if (!value)
	{
		throw std::runtime_error(ToStdString(value.error));
	}
}

void Expect(bool value, const char* message)
{
	if (!value)
	{
		throw std::runtime_error(message);
	}
}
} // namespace

int main()
{
	const auto root =
	    std::filesystem::current_path() / ("snake_editor_" + ToStdString(ResourceId::Generate().ToString()));
	try
	{
		std::filesystem::create_directories(root / "Assets");
		ComponentRegistry codecs;
		done(RegisterBuiltinComponents(codecs));
		done(Demo::RegisterSnakeComponents(codecs));
		Editor::ComponentEditors inspectors;
		done(Demo::RegisterSnakeEditors(inspectors));
		Expect(inspectors.Find("demo.SnakeController") && inspectors.Find("demo.SnakePresentation"),
		       "Snake property editors were not registered");
		Expect(!inspectors.Find("other.Component") && !inspectors.RegisterEditor("demo.SnakeController", {}),
		       "Unregistered or duplicate property editor handling changed");
		auto workspace = Take(Editor::ProjectWorkspace::Open(root));
		auto documents = Take(Editor::EditorDocuments::Create(*workspace));
		Demo::SnakeGame editGame;
		const Editor::EditContextSetup setup = [&editGame](ComponentLoadContext& context)
		{
			context.Provide(editGame);
			return Result<void>{};
		};
		const auto path = Take(VirtualPath::Parse("prj:Snake.ursascene"));
		auto document = Take(Editor::NewSceneDocument(path, ResourceKind::scene, codecs, documents->Session(), setup));
		auto draft = document->Payload<Editor::SceneDraft>();
		const NodeId node = draft->GetScene().RootNodes().front();
		auto controllerId = Take(draft->GetScene().AddComponent<Demo::SnakeController>(node, editGame, false, false));
		auto presentationId =
		    Take(draft->GetScene().AddComponent<Demo::SnakePresentation>(node, editGame, node, false));
		auto* controller = static_cast<Demo::SnakeController*>(draft->GetScene().GetComponent(controllerId));
		auto* presentation = static_cast<Demo::SnakePresentation*>(draft->GetScene().GetComponent(presentationId));
		controller->SetStressWindow(true);
		controller->SetSmokeGameplay(true);
		presentation->SetSmokeGameplay(true);
		document->MarkDirty();
		done(documents->RequestOpen(std::move(document)));
		done(documents->SaveCurrent());
		auto reopened = Take(Editor::OpenSceneDocument(path, ResourceKind::scene, codecs, documents->Session(), setup));
		const auto loaded = reopened->Payload<Editor::SceneDraft>();
		const NodeId loadedNode = loaded->GetScene().RootNodes().front();
		const auto* loadedController = loaded->GetScene().FindComponent<Demo::SnakeController>(loadedNode);
		const auto* loadedPresentation = loaded->GetScene().FindComponent<Demo::SnakePresentation>(loadedNode);
		Expect(loadedController && loadedController->StressWindow() && loadedController->SmokeGameplay() &&
		           loadedPresentation && loadedPresentation->SmokeGameplay() &&
		           loadedPresentation->RootNode() == loadedNode,
		       "Snake component fields did not survive edit/save/reopen");
		std::filesystem::remove_all(root);
		std::cout << "Snake Editor component test passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::filesystem::remove_all(root);
		std::cerr << error.what() << '\n';
		return 1;
	}
}

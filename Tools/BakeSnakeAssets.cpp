#include <SnakeGame.hpp>
#include <SnakeScene.hpp>
#include <SnakeSerialization.hpp>
#include <Core/StringInterop.hpp>
#include <Core/Log.hpp>
#include <Serialization/JsonResource.hpp>
#include <Serialization/PrefabResource.hpp>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>

namespace
{
Ursa::ResourceId StableId(uint8_t kind, uint32_t index)
{
	Ursa::ResourceId id;
	id.bytes[0] = 0x75;
	id.bytes[1] = 0x72;
	id.bytes[2] = 0x73;
	id.bytes[3] = 0x61;
	id.bytes[8] = kind;
	for (unsigned byte = 0; byte < 4; ++byte)
	{
		id.bytes[15 - byte] = static_cast<uint8_t>(index >> (byte * 8));
	}
	return id;
}

Ursa::Result<void> AssignStableIds(Ursa::Scene& scene)
{
	uint32_t nextNode = 1;
	uint32_t nextComponent = 1;
	std::function<Ursa::Result<void>(Ursa::NodeId)> visit = [&](Ursa::NodeId node) -> Ursa::Result<void>
	{
		auto assigned = scene.AssignNodePersistentId(node, StableId(1, nextNode++));
		if (!assigned)
		{
			return assigned;
		}
		auto components = scene.ComponentsOf(node);
		if (!components)
		{
			return Ursa::Result<void>::Failure(components.error);
		}
		for (Ursa::ComponentId component : components.value)
		{
			assigned = scene.AssignComponentPersistentId(component, StableId(2, nextComponent++));
			if (!assigned)
			{
				return assigned;
			}
		}
		auto children = scene.ChildrenOf(node);
		if (!children)
		{
			return Ursa::Result<void>::Failure(children.error);
		}
		for (Ursa::NodeId child : children.value)
		{
			assigned = visit(child);
			if (!assigned)
			{
				return assigned;
			}
		}
		return {};
	};
	for (Ursa::NodeId root : scene.RootNodes())
	{
		auto assigned = visit(root);
		if (!assigned)
		{
			return assigned;
		}
	}
	return {};
}

Ursa::Result<void> Bake(const std::filesystem::path& assetRoot, bool force)
{
	const auto directory = assetRoot / "Snake";
	const auto prefabPath = directory / "Board.ursaprefab";
	const auto scenePath = directory / "Snake.ursascene";
	if (!force && (std::filesystem::exists(prefabPath) || std::filesystem::exists(scenePath)))
	{
		return Ursa::Result<void>::Failure("Snake assets already exist; pass --force to replace them");
	}
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	if (error)
	{
		return Ursa::Result<void>::Failure("Cannot create Snake asset directory: " +
		                                   Ursa::ToUrsaString(error.message()));
	}

	Ursa::Demo::SnakeGame game;
	Ursa::Scene scene;
	auto prepared = Ursa::Demo::SetupSnakeScene(scene, game);
	if (!prepared)
	{
		return prepared;
	}
	auto assigned = AssignStableIds(scene);
	if (!assigned)
	{
		return assigned;
	}
	auto roots = scene.RootNodes();
	if (roots.size() != 1)
	{
		return Ursa::Result<void>::Failure("Snake scene must have one root");
	}
	auto children = scene.ChildrenOf(roots.front());
	if (!children || children.value.empty())
	{
		return Ursa::Result<void>::Failure("Snake scene has no board subtree");
	}
	const Ursa::NodeId board = children.value.front();
	auto parentId = scene.NodePersistentId(roots.front());
	if (!parentId)
	{
		return Ursa::Result<void>::Failure(parentId.error);
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
	const auto prefabId = StableId(3, 1);
	const auto sceneId = StableId(3, 2);
	const auto instanceId = StableId(3, 3);
	auto saved = Ursa::SavePrefabResource(prefabPath, prefabId, scene, board, registry);
	if (!saved)
	{
		return saved;
	}
	auto removed = scene.DestroyNode(board);
	if (!removed)
	{
		return removed;
	}
	auto encoded = Ursa::EncodeSceneDocument(scenePath, sceneId, scene, registry);
	if (!encoded)
	{
		return Ursa::Result<void>::Failure(encoded.error);
	}
	auto linked = Ursa::AddPrefabInstance(encoded.value, prefabId, instanceId, parentId.value);
	if (!linked)
	{
		return linked;
	}
	return Ursa::WriteJsonResource(scenePath, encoded.value);
}
} // namespace

int main(int argc, char** argv)
{
	Ursa::LogSession logging;
	if (argc < 2 || argc > 3 || (argc == 3 && std::string(argv[2]) != "--force"))
	{
		std::cerr << "Usage: UrsaSnakeAssetBake <project-directory> [--force]\n";
		return 2;
	}
	auto result = Bake(std::filesystem::u8path(argv[1]) / "Assets", argc == 3);
	if (!result)
	{
		URSA_LOG_ERROR("Demo", result.error);
		return 1;
	}
	URSA_LOG_INFO("Demo", "Baked Snake assets");
	return 0;
}

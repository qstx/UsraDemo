#include <Math/Math.hpp>
#include <Material/Material.hpp>
#include <Serialization/MaterialResource.hpp>
#include <Scene/GameObject.hpp>
#include <Scene/RenderingComponents.hpp>
#include <Shader/ShaderAsset.hpp>
#include <Vfs/MemoryFileProvider.hpp>
#include <memory>

int main()
{
	Ursa::Camera camera;
	camera.position = {0.0f, 12.0f, 18.0f};
	camera.target = {0.0f, 0.0f, 0.0f};
	const Ursa::Mat4 viewProjection = camera.ViewProjection(16.0f / 9.0f);
	const Ursa::Mat4 model = Ursa::Transform{{2.0f, 0.5f, 3.0f}, {1.0f, 1.0f, 1.0f}}.Matrix();
	const Ursa::Mat4 clipFromModel = viewProjection * model;
	auto object = Ursa::GameObject::Create("Header smoke");
	auto box = object->AddComponent<Ursa::BoxRenderComponent>();
	Ursa::Shader::ShaderAsset shader;
	shader.id = Ursa::ResourceId::Generate();
	Ursa::Shader::PropertyDeclaration tint;
	tint.identifier = "_Tint";
	tint.type = Ursa::Shader::PropertyType::Color;
	tint.defaultVector = {1.0f, 1.0f, 1.0f, 1.0f};
	shader.properties.push_back(tint);
	Ursa::MaterialInstance material(Ursa::MaterialType::Unlit);
	auto assigned = material.SetShader(shader.id);
	auto changed =
	    material.SetPropertyValue("_Tint", shader, Ursa::MaterialPropertyValue::ColorValue({0.3f, 0.5f, 0.7f, 1.0f}));
	Ursa::VirtualFileSystem filesystem;
	auto memory = std::make_shared<Ursa::MemoryFileProvider>();
	auto mounted = filesystem.MountProvider("prj", memory, Ursa::MountAccess::ReadWrite);
	auto path = Ursa::VirtualPath::Parse("prj:Materials/HeaderSmoke.ursamat");
	const auto materialId = Ursa::ResourceId::Generate();
	auto saved = mounted && path && assigned && changed
	                 ? Ursa::SaveMaterialResource(filesystem, path.value, materialId, material, {})
	                 : Ursa::Result<void>::Failure("Material Shader example setup failed");
	return clipFromModel.At(3, 3) > 0.0f && box && box.value->GetGameObject() == object &&
	               box.value->TryGetScene() == nullptr && saved
	           ? 0
	           : 1;
}

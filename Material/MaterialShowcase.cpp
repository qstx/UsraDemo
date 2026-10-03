#include <Core/Containers.hpp>
#include <Core/StringInterop.hpp>
#include <Application/Application.hpp>
#include <Core/Log.hpp>
#include <Material/Material.hpp>
#include <Scene/Scene.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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

Ursa::MaterialTexturePtr CheckerTexture()
{
	Ursa::Vector<uint8_t> pixels;
	pixels.reserve(8 * 8 * 4);
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			const bool light = (x / 2 + y / 2) % 2 == 0;
			pixels.push_back(light ? 255 : 40);
			pixels.push_back(light ? 230 : 70);
			pixels.push_back(light ? 190 : 160);
			pixels.push_back(255);
		}
	}
	return Take(Ursa::MaterialTexture::Create(8, 8, Ursa::TextureColorSpace::srgb, std::move(pixels)));
}

Ursa::MaterialTexturePtr BrushedNormal()
{
	Ursa::Vector<uint8_t> pixels;
	pixels.reserve(8 * 8 * 4);
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			pixels.push_back(static_cast<uint8_t>(x % 2 == 0 ? 145 : 110));
			pixels.push_back(128);
			pixels.push_back(254);
			pixels.push_back(255);
		}
	}
	return Take(Ursa::MaterialTexture::Create(8, 8, Ursa::TextureColorSpace::linear, std::move(pixels)));
}

class MaterialShowcase final : public Ursa::Application
{
private:
	Ursa::Result<void> OnStart() override
	{
		try
		{
			scene = std::make_shared<Ursa::Scene>();
			done(scene->SetAmbientLight({0.8f, 0.9f, 1.0f, 1.0f}, 0.3f));
			const auto cameraNode = Take(scene->CreateNode());
			Ursa::LocalTransform cameraPose;
			cameraPose.position = {0, 1.4f, 9};
			cameraPose.rotation = Ursa::Quat::FromAxisAngle({1, 0, 0}, -std::atan2(1.4f, 9.0f));
			done(scene->SetLocalTransform(cameraNode, cameraPose));
			done(scene->SetActiveCamera(Take(scene->AddComponent<Ursa::CameraComponent>(cameraNode))));

			const auto sunNode = Take(scene->CreateNode());
			const auto sunId = Take(scene->AddComponent<Ursa::DirectionalLightComponent>(sunNode));
			(void)sunId;
			done(scene->FindComponent<Ursa::DirectionalLightComponent>(sunNode)->SetIntensity(3.0f));
			const auto lampNode = Take(scene->CreateNode());
			Ursa::LocalTransform lampPose;
			lampPose.position = {2.0f, 2.5f, 3.0f};
			done(scene->SetLocalTransform(lampNode, lampPose));
			Take(scene->AddComponent<Ursa::PointLightComponent>(lampNode));
			auto* lamp = scene->FindComponent<Ursa::PointLightComponent>(lampNode);
			done(lamp->SetColor({1.0f, 0.6f, 0.25f, 1.0f}));
			done(lamp->SetIntensity(35.0f));
			done(lamp->SetRange(9.0f));

			auto unlit = std::make_shared<Ursa::MaterialInstance>(Ursa::MaterialType::unlit);
			done(unlit->SetTexture(Ursa::MaterialTextureSlot::baseColor, CheckerTexture()));
			done(unlit->SetSampler({false, true}));
			auto ceramic = std::make_shared<Ursa::MaterialInstance>(Ursa::MaterialType::pbrMetallicRoughness);
			done(ceramic->SetBaseColor({0.12f, 0.45f, 0.95f, 1.0f}));
			done(ceramic->SetMetallic(0.0f));
			done(ceramic->SetRoughness(0.75f));
			done(ceramic->SetTexture(Ursa::MaterialTextureSlot::normal, BrushedNormal()));
			auto gold = std::make_shared<Ursa::MaterialInstance>(Ursa::MaterialType::pbrMetallicRoughness);
			done(gold->SetBaseColor({0.95f, 0.55f, 0.16f, 1.0f}));
			done(gold->SetMetallic(1.0f));
			done(gold->SetRoughness(0.25f));
			done(gold->SetTexture(Ursa::MaterialTextureSlot::normal, BrushedNormal()));

			const auto addBox = [&](Ursa::Vec3 position, Ursa::Vec3 scale,
			                        std::shared_ptr<Ursa::MaterialInstance> material, Ursa::Color color = {})
			{
				const auto node = Take(scene->CreateNode());
				Ursa::LocalTransform pose;
				pose.position = position;
				pose.scale = scale;
				done(scene->SetLocalTransform(node, pose));
				Take(scene->AddComponent<Ursa::BoxRenderComponent>(node, color));
				scene->FindComponent<Ursa::BoxRenderComponent>(node)->material = std::move(material);
			};
			addBox({-2.2f, 0, 0}, {1.5f, 1.5f, 1.5f}, unlit);
			addBox({0, 0, 0}, {1.5f, 1.5f, 1.5f}, ceramic);
			addBox({2.2f, 0, 0}, {1.5f, 1.5f, 1.5f}, gold);
			addBox({0, -1.0f, -0.5f}, {8.0f, 0.2f, 3.0f}, {}, {0.13f, 0.17f, 0.22f, 1.0f});
			return {};
		}
		catch (const std::exception& error)
		{
			return Ursa::Result<void>::Failure(error.what());
		}
	}
};
} // namespace

int main(int argc, char** argv)
{
	Ursa::LogSession logging;
	URSA_LOG_INFO("Demo", "Starting UrsaMaterialShowcase");
	try
	{
		bool visible = true;
		uint32_t frameLimit = 0;
		for (int i = 1; i < argc; ++i)
		{
			const Ursa::String argument = argv[i];
			if (argument == "--hidden")
			{
				visible = false;
			}
			else if (argument == "--frames" && i + 1 < argc)
			{
				frameLimit = static_cast<uint32_t>(std::stoul(argv[++i]));
			}
			else
			{
				throw std::runtime_error("Usage: UrsaMaterialShowcase [--hidden] [--frames N]");
			}
		}
		MaterialShowcase app;
		done(app.Run(1000, 650, "Ursa Materials | Unlit checker / PBR ceramic / PBR metal", visible, frameLimit));
		URSA_LOG_INFO("Demo", "UrsaMaterialShowcase stopped");
		return 0;
	}
	catch (const std::exception& error)
	{
		URSA_LOG_CRITICAL("Demo", Ursa::String("UrsaMaterialShowcase: ") + error.what());
		return 1;
	}
}

#include <core/Containers.hpp>
#include <core/StringInterop.hpp>
#include <application/Application.hpp>
#include <core/Log.hpp>
#include <material/Material.hpp>
#include <scene/Scene.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
template <class T> T take(ursa::Result<T> result) {
    if (!result)
        throw std::runtime_error(ursa::toStdString(result.error));
    return std::move(result.value);
}

void done(ursa::Result<void> result) {
    if (!result)
        throw std::runtime_error(ursa::toStdString(result.error));
}

ursa::MaterialTexturePtr checkerTexture() {
    ursa::Vector<uint8_t> pixels;
    pixels.reserve(8 * 8 * 4);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            const bool light = (x / 2 + y / 2) % 2 == 0;
            pixels.push_back(light ? 255 : 40);
            pixels.push_back(light ? 230 : 70);
            pixels.push_back(light ? 190 : 160);
            pixels.push_back(255);
        }
    return take(ursa::MaterialTexture::create(8, 8, ursa::TextureColorSpace::Srgb, std::move(pixels)));
}

ursa::MaterialTexturePtr brushedNormal() {
    ursa::Vector<uint8_t> pixels;
    pixels.reserve(8 * 8 * 4);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            pixels.push_back(static_cast<uint8_t>(x % 2 == 0 ? 145 : 110));
            pixels.push_back(128);
            pixels.push_back(254);
            pixels.push_back(255);
        }
    return take(ursa::MaterialTexture::create(8, 8, ursa::TextureColorSpace::Linear, std::move(pixels)));
}

class MaterialShowcase final : public ursa::Application {
private:
    ursa::Result<void> onStart() override {
        try {
            scene_ = std::make_shared<ursa::Scene>();
            done(scene_->setAmbientLight({0.8f, 0.9f, 1.0f, 1.0f}, 0.3f));
            const auto cameraNode = take(scene_->createNode());
            ursa::LocalTransform cameraPose;
            cameraPose.position = {0, 1.4f, 9};
            cameraPose.rotation = ursa::Quat::fromAxisAngle({1, 0, 0}, -std::atan2(1.4f, 9.0f));
            done(scene_->setLocalTransform(cameraNode, cameraPose));
            done(scene_->setActiveCamera(take(scene_->addComponent<ursa::CameraComponent>(cameraNode))));

            const auto sunNode = take(scene_->createNode());
            const auto sunId = take(scene_->addComponent<ursa::DirectionalLightComponent>(sunNode));
            (void)sunId;
            done(scene_->findComponent<ursa::DirectionalLightComponent>(sunNode)->setIntensity(3.0f));
            const auto lampNode = take(scene_->createNode());
            ursa::LocalTransform lampPose;
            lampPose.position = {2.0f, 2.5f, 3.0f};
            done(scene_->setLocalTransform(lampNode, lampPose));
            take(scene_->addComponent<ursa::PointLightComponent>(lampNode));
            auto *lamp = scene_->findComponent<ursa::PointLightComponent>(lampNode);
            done(lamp->setColor({1.0f, 0.6f, 0.25f, 1.0f}));
            done(lamp->setIntensity(35.0f));
            done(lamp->setRange(9.0f));

            auto unlit = std::make_shared<ursa::MaterialInstance>(ursa::MaterialType::Unlit);
            done(unlit->setTexture(ursa::MaterialTextureSlot::BaseColor, checkerTexture()));
            done(unlit->setSampler({false, true}));
            auto ceramic = std::make_shared<ursa::MaterialInstance>(ursa::MaterialType::PbrMetallicRoughness);
            done(ceramic->setBaseColor({0.12f, 0.45f, 0.95f, 1.0f}));
            done(ceramic->setMetallic(0.0f));
            done(ceramic->setRoughness(0.75f));
            done(ceramic->setTexture(ursa::MaterialTextureSlot::Normal, brushedNormal()));
            auto gold = std::make_shared<ursa::MaterialInstance>(ursa::MaterialType::PbrMetallicRoughness);
            done(gold->setBaseColor({0.95f, 0.55f, 0.16f, 1.0f}));
            done(gold->setMetallic(1.0f));
            done(gold->setRoughness(0.25f));
            done(gold->setTexture(ursa::MaterialTextureSlot::Normal, brushedNormal()));

            const auto addBox = [&](ursa::Vec3 position, ursa::Vec3 scale,
                                    std::shared_ptr<ursa::MaterialInstance> material, ursa::Color color = {}) {
                const auto node = take(scene_->createNode());
                ursa::LocalTransform pose;
                pose.position = position;
                pose.scale = scale;
                done(scene_->setLocalTransform(node, pose));
                take(scene_->addComponent<ursa::BoxRenderComponent>(node, color));
                scene_->findComponent<ursa::BoxRenderComponent>(node)->material = std::move(material);
            };
            addBox({-2.2f, 0, 0}, {1.5f, 1.5f, 1.5f}, unlit);
            addBox({0, 0, 0}, {1.5f, 1.5f, 1.5f}, ceramic);
            addBox({2.2f, 0, 0}, {1.5f, 1.5f, 1.5f}, gold);
            addBox({0, -1.0f, -0.5f}, {8.0f, 0.2f, 3.0f}, {}, {0.13f, 0.17f, 0.22f, 1.0f});
            return {};
        } catch (const std::exception &error) {
            return ursa::Result<void>::failure(error.what());
        }
    }
};
} // namespace

int main(int argc, char **argv) {
    ursa::LogSession logging;
    URSA_LOG_INFO("Demo", "Starting UrsaMaterialShowcase");
    try {
        bool visible = true;
        uint32_t frameLimit = 0;
        for (int i = 1; i < argc; ++i) {
            const ursa::String argument = argv[i];
            if (argument == "--hidden")
                visible = false;
            else if (argument == "--frames" && i + 1 < argc)
                frameLimit = static_cast<uint32_t>(std::stoul(argv[++i]));
            else
                throw std::runtime_error("Usage: UrsaMaterialShowcase [--hidden] [--frames N]");
        }
        MaterialShowcase app;
        done(app.run(1000, 650, "Ursa Materials | Unlit checker / PBR ceramic / PBR metal", visible, frameLimit));
        URSA_LOG_INFO("Demo", "UrsaMaterialShowcase stopped");
        return 0;
    } catch (const std::exception &error) {
        URSA_LOG_CRITICAL("Demo", ursa::String("UrsaMaterialShowcase: ") + error.what());
        return 1;
    }
}

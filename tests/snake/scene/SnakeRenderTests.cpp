#include <core/StringInterop.hpp>
#include <core/Containers.hpp>
#include "SnakeScene.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
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

void writeBmp(const std::string &path, const ursa::Vector<uint8_t> &rgba, uint32_t width, uint32_t height) {
    const uint32_t fileSize = 54 + width * height * 4;
    ursa::Vector<uint8_t> header(54);
    const auto number = [&](size_t offset, uint32_t value) {
        for (int byte = 0; byte < 4; ++byte)
            header[offset + byte] = static_cast<uint8_t>(value >> (byte * 8));
    };
    header[0] = 'B';
    header[1] = 'M';
    number(2, fileSize);
    number(10, 54);
    number(14, 40);
    number(18, width);
    number(22, height);
    header[26] = 1;
    header[28] = 32;
    number(34, width * height * 4);
    std::ofstream output(path, std::ios::binary);
    if (!output)
        throw std::runtime_error("Cannot open preview image");
    output.write(reinterpret_cast<const char *>(header.data()), static_cast<std::streamsize>(header.size()));
    for (int y = static_cast<int>(height) - 1; y >= 0; --y)
        for (uint32_t x = 0; x < width; ++x) {
            const size_t offset = (static_cast<size_t>(y) * width + x) * 4;
            const char bgra[4]{static_cast<char>(rgba[offset + 2]), static_cast<char>(rgba[offset + 1]),
                               static_cast<char>(rgba[offset]), static_cast<char>(rgba[offset + 3])};
            output.write(bgra, 4);
        }
    if (!output)
        throw std::runtime_error("Cannot write preview image");
}
} // namespace

int main(int argc, char **argv) {
    try {
        constexpr uint32_t width = 800;
        constexpr uint32_t height = 450;
        ursa::demo::SnakeConfig config;
        config.initialFood = ursa::demo::Cell{10, 8};
        ursa::demo::SnakeGame game(config);
        ursa::rhi::DeviceDesc desc;
#ifndef NDEBUG
        desc.validation = true;
#endif
        auto device = take(ursa::rhi::createDevice(desc));
        auto renderer = take(ursa::Renderer3D::create(device));
        auto color = take(device->createTexture(
            {width, height, ursa::rhi::Format::RGBA8Unorm,
             ursa::rhi::TextureUsage::ColorAttachment | ursa::rhi::TextureUsage::TransferSource}));
        ursa::Scene scene;
        done(ursa::demo::setupSnakeScene(scene, game));
        scene.beginFrame();
        done(scene.lateUpdate({}));
        const auto data = take(scene.collectRenderData(static_cast<float>(width) / height));
        done(take(renderer->renderOffscreenModels(color, data.viewProjection, data.boxes))->wait());
        auto readback =
            take(device->createBuffer({static_cast<size_t>(width) * height * 4,
                                       ursa::rhi::BufferUsage::TransferDestination, ursa::rhi::Memory::Readback}));
        auto command = take(device->createCommandList());
        done(command->begin());
        done(command->barrier(readback, ursa::rhi::ResourceState::Undefined,
                              ursa::rhi::ResourceState::TransferDestination));
        done(command->copyTextureToBuffer(color, readback));
        done(command->end());
        done(take(device->submit(command))->wait());
        ursa::Vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
        done(readback->read(0, pixels.data(), pixels.size()));
        size_t snakePixels = 0;
        size_t foodPixels = 0;
        for (size_t i = 0; i < pixels.size(); i += 4) {
            if (pixels[i + 1] > 80 && pixels[i + 1] > pixels[i] * 1.3f && pixels[i + 1] > pixels[i + 2] * 1.2f)
                ++snakePixels;
            if (pixels[i] > 90 && pixels[i] > pixels[i + 1] * 1.5f && pixels[i] > pixels[i + 2] * 2.0f)
                ++foodPixels;
        }
        if (snakePixels < 100 || foodPixels < 20)
            throw std::runtime_error("Rendered snake or food pixels were missing");
        if (argc == 2)
            writeBmp(argv[1], pixels, width, height);
        renderer.reset();
        done(device->waitIdle());
        std::cout << "Snake render test passed: " << snakePixels << " snake pixels, " << foodPixels << " food pixels\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Snake render test failed: " << error.what() << '\n';
        return 1;
    }
}

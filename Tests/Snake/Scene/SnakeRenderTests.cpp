#include <Core/Types.hpp>
#include <Core/StringInterop.hpp>
#include <Core/Containers.hpp>
#include "SnakeScene.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

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

void WriteBmp(const std::string& path, const Ursa::Vector<Ursa::UInt8>& rgba, Ursa::UInt32 width, Ursa::UInt32 height)
{
	const Ursa::UInt32 fileSize = 54 + width * height * 4;
	Ursa::Vector<Ursa::UInt8> header(54);
	const auto number = [&](Ursa::Size offset, Ursa::UInt32 value)
	{
		for (int byte = 0; byte < 4; ++byte)
		{
			header[offset + byte] = static_cast<Ursa::UInt8>(value >> (byte * 8));
		}
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
	{
		throw std::runtime_error("Cannot open preview image");
	}
	output.write(reinterpret_cast<const char*>(header.data()), static_cast<std::streamsize>(header.size()));
	for (int y = static_cast<int>(height) - 1; y >= 0; --y)
	{
		for (Ursa::UInt32 x = 0; x < width; ++x)
		{
			const Ursa::Size offset = (static_cast<Ursa::Size>(y) * width + x) * 4;
			const char bgra[4]{static_cast<char>(rgba[offset + 2]), static_cast<char>(rgba[offset + 1]),
			                   static_cast<char>(rgba[offset]), static_cast<char>(rgba[offset + 3])};
			output.write(bgra, 4);
		}
	}
	if (!output)
	{
		throw std::runtime_error("Cannot write preview image");
	}
}
} // namespace

int main(int argc, char** argv)
{
	try
	{
		constexpr Ursa::UInt32 width = 800;
		constexpr Ursa::UInt32 height = 450;
		Ursa::Demo::SnakeConfig config;
		config.initialFood = Ursa::Demo::Cell{10, 8};
		Ursa::Demo::SnakeGame game(config);
		Ursa::Rhi::DeviceDesc desc;
#ifndef NDEBUG
		desc.validation = true;
#endif
		auto device = Take(Ursa::Rhi::CreateDevice(desc));
		auto renderer = Take(Ursa::Renderer3D::Create(device));
		auto color = Take(device->CreateTexture(
		    {width, height, Ursa::Rhi::Format::RgbA8Unorm,
		     Ursa::Rhi::TextureUsage::ColorAttachment | Ursa::Rhi::TextureUsage::TransferSource}));
		Ursa::Scene scene;
		done(Ursa::Demo::SetupSnakeScene(scene, game));
		scene.BeginFrame();
		done(scene.LateUpdate({}));
		const auto data = Take(scene.CollectRenderData(static_cast<Ursa::Float32>(width) / height));
		done(Take(renderer->RenderOffscreenModels(color, data.viewProjection, data.boxes))->Wait());
		auto readback =
		    Take(device->CreateBuffer({static_cast<Ursa::Size>(width) * height * 4,
		                               Ursa::Rhi::BufferUsage::TransferDestination, Ursa::Rhi::Memory::Readback}));
		auto command = Take(device->CreateCommandList());
		done(command->Begin());
		done(command->Barrier(readback, Ursa::Rhi::ResourceState::Undefined,
		                      Ursa::Rhi::ResourceState::TransferDestination));
		done(command->CopyTextureToBuffer(color, readback));
		done(command->End());
		done(Take(device->Submit(command))->Wait());
		Ursa::Vector<Ursa::UInt8> pixels(static_cast<Ursa::Size>(width) * height * 4);
		done(readback->Read(0, pixels.data(), pixels.size()));
		Ursa::Size snakePixels = 0;
		Ursa::Size foodPixels = 0;
		for (Ursa::Size i = 0; i < pixels.size(); i += 4)
		{
			if (pixels[i + 1] > 80 && pixels[i + 1] > pixels[i] * 1.3f && pixels[i + 1] > pixels[i + 2] * 1.2f)
			{
				++snakePixels;
			}
			if (pixels[i] > 90 && pixels[i] > pixels[i + 1] * 1.5f && pixels[i] > pixels[i + 2] * 2.0f)
			{
				++foodPixels;
			}
		}
		if (snakePixels < 100 || foodPixels < 20)
		{
			throw std::runtime_error("Rendered snake or food pixels were missing");
		}
		if (argc == 2)
		{
			WriteBmp(argv[1], pixels, width, height);
		}
		renderer.reset();
		done(device->WaitIdle());
		std::cout << "Snake render test passed: " << snakePixels << " snake pixels, " << foodPixels << " food pixels\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "Snake render test failed: " << error.what() << '\n';
		return 1;
	}
}

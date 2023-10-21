#include "Application.h"
#include <platform/PlatformSpecific.h>
#include <render/RenderInterface.h>

#include <filesystem/VirtualFileSystemManager.h>
#include <filesystem/native/NativeFileSystem.h>
#include <filesystem/vfs_helper.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <interface/SharedStructure.h>


using namespace engine;

void get_platform_specific_window_data(FRenderInterfaceCreateInfo& rhi_ci, const std::unique_ptr<IWindowInterface>& window)
{
#if defined(PLATFORM_WIN32)
	auto platform_window = static_cast<platform::CWindowWin32*>(window.get());
	platform_window->getPlatformData(rhi_ci.hinstance, rhi_ci.hwnd);
#endif
}

void CApplication::create()
{
	vfs = std::make_unique<filesystem::CVirtualFileSystemManager>();

	vfs->mount("/root", std::make_unique<filesystem::CNativeFileSystem>(""));

	FWindowCreateInfo window_ci{};
	window_ci.name = "Vulkan App";
	window_ci.flags = WINDOW_RESIZABLE_FLAG_BIT;
	window_ci.width = 800;
	window_ci.height = 600;

	/*std::ifstream f("test_shader.frag.spv", std::ios::in | std::ios::binary);
	std::vector<char> bytes{ std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };

	if (bytes.size() % sizeof(uint32_t) != 0) {
		std::cerr << "Data in not aligned uint32_t" << std::endl;
		return;
	}

	std::vector<uint32_t> data(bytes.size() / sizeof(uint32_t));
	std::memcpy(data.data(), bytes.data(), bytes.size());

	std::stringstream ss{};
	ss << "const std::vector<uint32_t> fragment_shader {";
	for (auto& n : data)
		ss << n << ", ";
	ss << "};";
	std::cout << ss.str();*/

	window_handle = make_platform_window(window_ci);

	FRenderInterfaceCreateInfo rhi_ci{};
	rhi_ci.render_api = ERenderApi::eVulkan11;
	window_handle->getWindowSize(rhi_ci.initialWidth, rhi_ci.initialHeight);
	get_platform_specific_window_data(rhi_ci, window_handle);

	render_interface = std::make_unique<render::CRenderInterface>();
	render_interface->create(rhi_ci);

	std::vector<uint8_t> render_config{};
	vfs->readFile("/root/render_config.as", render_config);

	render_interface->createRendererBehavior(render_config);

	//loadTexture("/root/skybox.ktx2");
	//loadTexture("/root/3dtex_1_reference_u.ktx2");
}

FResourceView CApplication::loadTexture(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::const_hash(ext.c_str(), ext.size());

	std::vector<uint8_t> data{};
	vfs->readFile(path, data);

	return render_interface->createTexture(name, data, static_cast<EMimeType>(ext_hash));
}

FResourceView CApplication::loadShader(const std::string& name)
{
	return FResourceView{};
}

void CApplication::loop()
{
	while (window_handle->process())
	{
		if (window_handle->isResized())
		{
			// Wait while window is minimized
			while (window_handle->isMinimized())
			{
				if (!window_handle->process())
					return;
			}

			int32_t window_width{}, window_height{};
			window_handle->getWindowSize(window_width, window_height);
			render_interface->updateSurfaceSize(window_width, window_height);
		}

		render_interface->beginFrame();
		render_interface->endFrame();
	}
}
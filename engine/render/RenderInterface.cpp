#include <RenderInterface.h>

#include <render_api/vulkan/VkRenderHandle.h>
#include <shader_compiler/compiler_util.h>

using namespace engine;
using namespace engine::render;

void copy_platform_specific_create_data(engine::FRenderContextCreateInfo& context_ci, const engine::FRenderInterfaceCreateInfo& rhi_ci)
{
#if defined(PLATFORM_WIN32)
	context_ci.hinstance = rhi_ci.hinstance;
	context_ci.hwnd = rhi_ci.hwnd;
#endif
}

std::unique_ptr<engine::IRenderHandleInterface> make_render_handle_interface(const engine::FRenderInterfaceCreateInfo& rhi_ci)
{
	engine::FRenderContextCreateInfo context_ci{};
	std::unique_ptr<engine::IRenderHandleInterface> render_handle{};

	if (rhi_ci.render_api == engine::ERenderApi::eVulkan10 || rhi_ci.render_api == engine::ERenderApi::eVulkan11 ||
		rhi_ci.render_api == engine::ERenderApi::eVulkan12 || rhi_ci.render_api == engine::ERenderApi::eVulkan13)
	{
		render_handle = std::make_unique<vulkan::CVkRenderHandle>();

		context_ci.apiVersion = rhi_ci.render_api;
		context_ci.requiredDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
			VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
			VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME
		};
		
		context_ci.optionalDeviceExtensions = {
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

#ifndef BUILD_WITHOUT_DEBUGGIND
		context_ci.validationState = true;
		context_ci.validationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
		context_ci.validationFeatures = vulkan::SYNCHRONIZATION_VALIDATION_FLAG_BIT;
#endif

		context_ci.initialWidth = rhi_ci.initialWidth;
		context_ci.initialHeight = rhi_ci.initialHeight;

		copy_platform_specific_create_data(context_ci, rhi_ci);

		render_handle->create(context_ci);
	}

	return render_handle;
}

struct FShaderNameInfo
{
	std::string library_name;
	std::string shader_name;
	std::vector<std::string> definitions;
};

FShaderNameInfo get_shader_name_info(const std::string& shader_name)
{
	FShaderNameInfo name_info{};

	std::string token;
	std::stringstream ss(shader_name);

	if (std::getline(ss, token, ':'))
		name_info.library_name = token;

	if (std::getline(ss, token, ':'))
		name_info.shader_name = token;

	while (std::getline(ss, token, ':'))
		name_info.definitions.emplace_back(token);

	return name_info;
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	resourceLoader = std::make_unique<CResourceLoader>();
	renderHandle = make_render_handle_interface(rhi_ci);

	renderScriptCore = std::make_unique<CRenderScriptCore>();
	renderScriptCore->create(this, renderHandle.get());
}

void CRenderInterface::createRendererBehavior(const std::vector<uint8_t>& script)
{
	auto render_config_executor = renderScriptCore->build("render_config", script);
	auto result = render_config_executor->run();
	return;
}

FResourceView CRenderInterface::createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime)
{
	FTextureCreateInfo texture_ci{};
	FTextureUploadInfo texture_ui{};
	resourceLoader->loadTexture(data, mime, texture_ci, texture_ui);

	return renderHandle->createTexture(name, texture_ci, std::move(texture_ui));
}

FResourceView CRenderInterface::createShader(const FShaderNameLoadInfo& name_info, const FGraphicsShaderCreateInfo& create_info)
{
	return createShader<FGraphicsShaderCreateInfo>(name_info, create_info);
}

FResourceView CRenderInterface::createShader(const FShaderNameLoadInfo& name_info, const FComputeShaderCreateInfo& create_info)
{
	return createShader<FComputeShaderCreateInfo>(name_info, create_info);
}

void CRenderInterface::updateSurfaceSize(int32_t width, int32_t height)
{
	renderHandle->updateSurface(width, height);
}

void CRenderInterface::beginFrame()
{
	renderHandle->beginFrameCommandBuffers();
}

void CRenderInterface::endFrame()
{
	renderHandle->endFrameCommandBuffers();
}

FShaderNameLoadInfo CRenderInterface::getShaderNameTag(const std::string& name)
{
	auto name_info = get_shader_name_info(name);

	FShaderNameLoadInfo load_info{};
	load_info.libraryId = utl::fnv1a_64_hash(name_info.library_name.c_str());
	load_info.shaderId = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);
	load_info.originalName = name;

	return load_info;
}

void CRenderInterface::createRendering(const FRenderingCreateInfo& rendering_ci)
{
	// Create CRenderStages here. Load script and create executable.
	return;
}
#include "RenderInterface.h"

#include <vulkan/VkRenderHandle.h>

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
			VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME
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

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	render_handle = make_render_handle_interface(rhi_ci);
}

void CRenderInterface::updateSurfaceSize(int32_t width, int32_t height)
{
	render_handle->updateSurface(width, height);
}

void CRenderInterface::beginFrame()
{
	render_handle->beginFrameCommandBuffers();
}

void CRenderInterface::endFrame()
{
	render_handle->endFrameCommandBuffers();
}

void CRenderInterface::processMessage(const FRenderMessage& message)
{

}
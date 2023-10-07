#pragma once

#if defined( PLATFORM_WIN32 )
#include <Windows.h>
#endif

#include <string>
#include <vector>

namespace engine
{
	enum class ERenderApi
	{
		eVulkan10,
		eVulkan11,
		eVulkan12,
		eVulkan13,
		eOpenGL,
		eDirectX12
	};

	struct FRenderInterfaceCreateInfo
	{
		ERenderApi render_api{};

#if defined( PLATFORM_WIN32 )
		HINSTANCE hinstance;
		HWND hwnd;
#endif

		int32_t initialWidth{};
		int32_t initialHeight{};
	};

	struct FRenderContextCreateInfo
	{
		std::string appName{ "dummy_app" };
		ERenderApi apiVersion{};

#if defined( VK_USE_PLATFORM_WIN32_KHR )
		HINSTANCE                       hinstance;
		HWND                            hwnd;
#endif
		std::vector<char> pipelineCache{};

		std::vector<std::string> requiredDeviceExtensions{};
		std::vector<std::string> optionalDeviceExtensions{};

		uint32_t initialWidth{ 0u };
		uint32_t initialHeight{ 0u };

		bool validationState{ false };
		uint32_t validationFeatures{ 0u };
		std::vector<std::string> validationLayers{};
	};

	enum ERenderMessageType
	{
		RENDER_MESSAGE_INITIALIZE,

	};

	struct FRenderMessage
	{

	};

	class IRenderInterface
	{
	public:
		virtual ~IRenderInterface() = default;

		virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;

		virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
	};

	class IRenderHandleInterface
	{
	public:
		virtual ~IRenderHandleInterface() = default;

		virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
		
		virtual void syncGPUIdle() = 0;
		virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

		virtual void beginFrameCommandBuffers() = 0;
		virtual void endFrameCommandBuffers() = 0;
	};
}
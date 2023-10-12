#pragma once

#if defined( PLATFORM_WIN32 )
#include <Windows.h>
#endif

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "SharedStructure.h"

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

	enum ETextureImageTypeFlagBits
	{
		TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT = 1 << 0,
		TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT = 1 << 1,
		TEXTURE_IMAGE_TYPE_1D_FLAG_BIT = 1 << 2,
		TEXTURE_IMAGE_TYPE_2D_FLAG_BIT = 1 << 3,
		TEXTURE_IMAGE_TYPE_3D_FLAG_BIT = 1 << 4
	};

	enum class EPrimitiveTopology
	{
		ePointList,
		eLineStrip,
		eTriangleList,
		eTriangleStrip,
		ePatchList
	};

	enum class ECullingMode
	{
		eNone,
		eBackFace,
		eFrontFace,
		eDoubleSided
	};

	enum class EFrontFace
	{
		eClockwise,
		eCounterClockwise
	};

	enum class EShaderStage
	{
		eVertexShader,
		eControlShader,
		eEvaluationShader,
		eGeometryShader,
		eFragmentShader,
		eComputeShader
	};

	struct FVertexBindingDescription
	{
		uint32_t binding{};
		uint32_t stride{};
	};

	struct FVertexAttributeDescription
	{
		uint32_t elementSize{};
		uint32_t stride{};

		uint32_t binding{};
		uint32_t location{};
		uint32_t offset{};
	};

	struct FVertexDeclaration
	{
		std::vector<FVertexBindingDescription> bindingDescriptions{};
		std::vector<FVertexAttributeDescription> attributeDescriptions{};
	};

	struct FGraphicsShaderCreateInfo
	{
		uint32_t usageCoutn{ 1u };

		EPrimitiveTopology primitiveTopology{};
		ECullingMode cullingMode{};
		EFrontFace frontFace{};

		// TODO: add multisampling info

		std::vector<size_t> colorAttachments{};
		size_t depthAttachment{ ~0ull };

		FVertexDeclaration vertexDeclaration{};

		std::unordered_map<EShaderStage, std::vector<uint32_t>> shaderStageCode{};
	};

	struct FTextureCreateInfo
	{
		std::string name{};

		uint32_t format{};
		uint32_t typeFlags{};

		uint32_t width{};
		uint32_t height{};
		uint32_t depth{};

		uint32_t mipLevels{ 1u };
		uint32_t arrayLayers{ 1u };

		std::vector<std::vector<size_t>> offsets{};
		std::vector<uint8_t> data{};

		bool result{ false };
	};

	class IRenderInterface
	{
	public:
		virtual ~IRenderInterface() = default;

		virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
		virtual size_t createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) = 0;

		virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
	};

	class IRenderHandleInterface
	{
	public:
		virtual ~IRenderHandleInterface() = default;

		virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
		virtual size_t createTexture(const FTextureCreateInfo& texture_ci) = 0;
		
		virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

		virtual void beginFrameCommandBuffers() = 0;
		virtual void endFrameCommandBuffers() = 0;
	};
}
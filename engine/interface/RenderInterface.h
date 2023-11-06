#pragma once

#if defined( PLATFORM_WIN32 )
#include <Windows.h>
#endif

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "SharedStructure.h"

//#include <shader_compiler/compiler_util.h>

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

	enum EResourceTypeBits : uint32_t
	{
		RESOURCE_TYPE_TEXTURE_BIT = 1 << 0,
		RESOURCE_TYPE_RENDER_TARGET_BIT = 1 << 1,
		RESOURCE_TYPE_BUFFER_BIT = 1 << 2,
		RESOURCE_TYPE_SHADER_BIT = 1 << 3
	};

	enum ETextureImageTypeFlagBits : uint32_t
	{
		TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT = 1 << 0,
		TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT = 1 << 1,
		TEXTURE_IMAGE_TYPE_1D_FLAG_BIT = 1 << 2,
		TEXTURE_IMAGE_TYPE_2D_FLAG_BIT = 1 << 3,
		TEXTURE_IMAGE_TYPE_3D_FLAG_BIT = 1 << 4,
		TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT = 1 << 5,
		TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT = 1 << 6,
		TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT = 1 << 7,
		TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT = 1 << 8
	};

	enum EQueueFamilyUsageFlagBits : uint32_t
	{
		QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT = 1 << 0,
		QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT = 1 << 1,
		QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT = 1 << 2,
		QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT = 1 << 3
	};

	enum ETextureFormat : uint32_t
	{
		FORMAT_RG16_SFLOAT,
		FORMAT_RGBA8_SRGB,
		FORMAT_RGBA8_UNORM,
		FORMAT_RGBA16_SFLOAT,
		FORMAT_RGBA32_SFLOAT 
	};

	enum ERenderingType
	{
		RENDERING_TYPE_CASCADED_SHADOWS,
		RENDERING_TYPE_DIRECTIONAL_SHADOWS,
		RENDERING_TYPE_OMNI_SHADOWS,
		RENDERING_TYPE_COMPUTE,
		RENDERING_TYPE_ZBUFFER,
		RENDERING_TYPE_SCREENSPACE,
		RENDERING_TYPE_BRDFLUT_GENERATOR,
		RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR,
		RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR
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

	struct FResourceView
	{
		size_t id{ ~0ull };
		uint32_t mask{};
	};

	struct FTextureUploadInfo
	{
		size_t textureId{ ~0ull };
		std::vector<uint8_t> data{};
		std::vector<std::vector<size_t>> mipLayerOffsets{};
	};

	struct FTextureCreateInfo
	{
		bool isSwapchainTarget{ false };

		uint32_t format{ 0u };
		uint32_t typeFlags{ 0u };

		uint32_t width{ 1u };
		uint32_t height{ 1u };
		uint32_t depth{ 1u };

		uint32_t mipLevels{ 1u };
		uint32_t arrayLayers{ 1u };
	};

	struct FReflectionUniform
	{
		uint32_t set{ 0u };
		uint32_t binding{ 0u };
		size_t offset{ 0ull };
		size_t size{ 0ull };
	};

	template<class _Ty>
	struct FReflectionUniformBlock
	{
		uint32_t set{ 0u };
		uint32_t binding{ 0u };
		size_t size{ 0ull };
		std::unordered_map<size_t, _Ty> uniforms{};
	};

	template<class _Ty>
	struct FReflectionPushConstantBlock
	{
		size_t size{ 0ull };
		std::unordered_map<size_t, _Ty> uniforms{};
	};

	struct FShaderReflectionData
	{
		std::unordered_map<size_t, FReflectionUniform> uniforms{};
		std::unordered_map<size_t, FReflectionUniformBlock<FReflectionUniform>> uniformBlocks{};
		std::unordered_map<size_t, FReflectionPushConstantBlock<FReflectionUniform>> pushBlocks{};
	};

	template<class _Ty>
	struct FPoint2D
	{
		_Ty x{ 0 }, y{ 0 };
	};

	template<class _Ty>
	struct FRect2D
	{
		FPoint2D<_Ty> offset{};
		FPoint2D<_Ty> size{};
	};

	struct FShaderNameLoadInfo
	{
		size_t libraryId{ ~0ull };
		size_t shaderId{ ~0ull };
		std::string originalName{};
	};

	struct FRenderingCreateInfo
	{
		uint32_t queue{ 0u };
		std::string renderScript{};

		FShaderNameLoadInfo shader{};

		bool oneTime{ false };
		bool flipViewport{ false };

		std::vector<FResourceView> shaderInputs{};
		std::vector<FResourceView> renderTargets{};

		FResourceView depthTarget{};
		FResourceView stencilTarget{};
	};

	struct FBeginRenderingInfo
	{
		uint32_t queue{ 0u };
		FRect2D<float> scissorRect{};
		FRect2D<float> renderArea{};
		bool flipViewport{ false };

		std::vector<FResourceView> colorAttachments{};
		FResourceView depthAttachment{};
		FResourceView stencilAttachment{};
	};

	enum EShaderPrimitiveTopology : uint32_t
	{
		SHADER_PRIMITIVE_TOPOLOGY_POINT_LIST,
		SHADER_PRIMITIVE_TOPOLOGY_LINE_LIST,
		SHADER_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	};

	enum EShaderCullMode : uint32_t
	{
		SHADER_CULL_MODE_NONE = 1 << 0,
		SHADER_CULL_MODE_FRONT_BIT = 1 << 1,
		SHADER_CULL_MODE_BACK_BIT = 1 << 2,
		SHADER_CULL_MODE_FRONT_AND_BACK = SHADER_CULL_MODE_FRONT_BIT | SHADER_CULL_MODE_BACK_BIT
	};

	enum EShaderFrontFace : uint32_t
	{
		SHADER_FRONT_FACE_CCW,
		SHADER_FRONT_FACE_CW
	};

	struct FVertexBindingDescription
	{
		uint32_t binding{};
		uint32_t stride{};
	};

	struct FVertexAttributeDescription
	{
		uint32_t location{};
		uint32_t binding{};
		uint32_t format{};
		uint32_t offset{};
	};

	struct FVertexDeclarationInfo
	{
		std::vector<FVertexBindingDescription> bindingDescriptions{};
		std::vector<FVertexAttributeDescription> attributeDescriptions{};
	};

	struct FShaderCreationBlockInfo
	{
		uint32_t stage{ 0u };
		std::vector<uint32_t>* code{ nullptr };
	};

	struct FGraphicsShaderCreateInfo
	{
		uint32_t primitiveTopology{};
		uint32_t cullMode{};
		uint32_t frontFace{};

		uint32_t usageCount{ 1u };

		std::vector<FResourceView> colorAttachments{};
		FResourceView depthAttachment{};
		FResourceView stencilAttachment{};

		FVertexDeclarationInfo vertexDeclaration{};
	};

	struct FComputeShaderCreateInfo
	{
		uint32_t usageCount{ 1u };
	};

	class IRenderInterface
	{
	public:
		virtual ~IRenderInterface() = default;

		virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
		virtual void createRendererBehavior(const std::vector<uint8_t>& script) = 0;
		virtual FResourceView createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) = 0;

		virtual FResourceView createShader(const FShaderNameLoadInfo& name_info, const FGraphicsShaderCreateInfo& create_info) = 0;
		virtual FResourceView createShader(const FShaderNameLoadInfo& name_info, const FComputeShaderCreateInfo& create_info) = 0;

		virtual FShaderNameLoadInfo getShaderNameTag(const std::string& name) = 0;
		virtual void createRendering(const FRenderingCreateInfo& rendering_ci) = 0;

		virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;
	};

	class IRenderHandleInterface
	{
	public:
		virtual ~IRenderHandleInterface() = default;

		virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
		virtual FResourceView createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) = 0;
		virtual FResourceView createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;

		virtual FResourceView createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
		virtual FResourceView createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;

		virtual FShaderReflectionData getShaderReflection(const FResourceView& shader_view) = 0;

		virtual FResourceView getResource(const char* name, uint32_t resource_type) = 0;

		virtual FPoint2D<uint32_t> getRenderResolution() = 0;

		virtual uint32_t getDepthFormat() = 0;
		virtual uint32_t getApiFormat(uint32_t global_format) = 0;
		virtual uint32_t getPresentFormat() = 0;
		
		virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

		virtual void beginFrameCommandBuffers() = 0;
		virtual void endFrameCommandBuffers() = 0;
	};
}
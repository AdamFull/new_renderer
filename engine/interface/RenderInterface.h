#pragma once

#if defined( PLATFORM_WIN32 )
#include <Windows.h>
#endif

#include <glm/glm.hpp>

#include <set>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "SharedStructure.h"

//#include <shader_compiler/compiler_util.h>

namespace engine
{
	namespace render
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

		enum EShaderResourcesUpdateFlagBits
		{
			SHADER_RESOURCES_UPDATE_UNIFORM_BLOCKS_FLAG_BIT = 1 << 0,
			SHADER_RESOURCES_UPDATE_TEXTURES_FLAG_BIT = 1 << 1,
			SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT = 
													SHADER_RESOURCES_UPDATE_UNIFORM_BLOCKS_FLAG_BIT | 
													SHADER_RESOURCES_UPDATE_TEXTURES_FLAG_BIT
		};

		enum ERenderingStepType
		{
			RENDERING_STEP_SYNCHRONIZATION_TYPE,
			RENDERING_STEP_RENDERING_TYPE
		};

		enum ERenderingType
		{
			RENDERING_FULLSCREEN_TYPE,
			RENDERING_GEOMETRY_TYPE,
			RENDERING_RENDER_SCRIPT_TYPE
		};

		enum ESynchronizationType
		{
			SYNCHRONIZATION_WAIT_TYPE,
			SYNCHRONIZATION_SIGNAL_TYPE
		};

		enum class EAlphaMode
		{
			EOPAQUE,
			EMASK,
			EBLEND
		};

		enum class EVertexAttributeType : uint32_t
		{
			ePosition,
			eColor,
			eTexCoord,
			eNormal,
			eTangent,
			eJoint,
			eWeight,
			eIndex
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

		//struct FResourceView
		//{
		//	size_t id{ ~0ull };
		//	uint32_t mask{};
		//};

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
			FPoint2D() = default;
			FPoint2D(_Ty val) : x(val), y(val) {}
			FPoint2D(_Ty _x, _Ty _y) : x(_x), y(_y) {}

			_Ty x{ 0 }, y{ 0 };
		};

		template<class _Ty>
		struct FRect2D
		{
			FRect2D() = default;
			FRect2D(const FPoint2D<_Ty>& _offset, const FPoint2D<_Ty>& _size) : offset(_offset), size(_size) {}
			FRect2D(_Ty offset_x, _Ty offset_y, _Ty size_x, _Ty size_y) : offset(offset_x, offset_y), size(size_x, size_y) {}

			FPoint2D<_Ty> offset{};
			FPoint2D<_Ty> size{};
		};

		struct FShaderUploadInfo
		{
			size_t libraryId{ ~0ull };
			size_t shaderId{ ~0ull };
			std::string originalName{};
		};

		struct FRenderingStepSynchronizationInfo
		{
			uint32_t type{};

			uint32_t queueFrom{};
			uint32_t queueTo{};
		};

		struct FRenderingStepRenderingInfo
		{
			uint32_t type{ ~0u };

			uint32_t queue{ 0u };

			std::string renderScript{}; // Optional

			std::string shaderName{}; // Optional

			bool oneTime{ false }; // Destroy after execution
			bool flipViewport{ false }; // Flip viewport horizontally
			bool materialPass{ false };

			std::unordered_map<std::string, size_t> shaderInputs{};
			std::vector<size_t> renderTargets{};

			size_t depthTarget{ ~0ull };
			size_t stencilTarget{ ~0ull };
		};

		struct FRenderingStepCreateInfo
		{
			uint32_t type{};

			FRenderingStepSynchronizationInfo synchronization{};
			FRenderingStepRenderingInfo rendering{};
		};

		struct FBeginRenderingInfo
		{
			uint32_t queue{ 0u };
			FRect2D<float> renderArea{};
			bool flipViewport{ false };

			std::vector<size_t> colorAttachments{};
			size_t depthAttachment{ ~0ull };
			size_t stencilAttachment{ ~0ull };
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

			std::vector<size_t> colorAttachments{};
			size_t depthAttachment{ ~0ull };
			size_t stencilAttachment{ ~0ull };

			FVertexDeclarationInfo vertexDeclaration{};
		};

		struct FComputeShaderCreateInfo
		{
			uint32_t usageCount{ 1u };
		};

		struct FUniformDataView
		{
			const void* data{ nullptr };
			size_t size{ 0ull };
		};

		struct FShaderResourceCommit
		{
			size_t shaderId{ ~0ull };
			std::unordered_map<size_t, FUniformDataView> uniforms{};
			std::unordered_map<size_t, size_t> textures{};
		};

		struct FMaterial
		{
			// Base params
			EAlphaMode alphaMode{ EAlphaMode::EOPAQUE };
			float alphaCutoff{ 0.5f };
			glm::vec3 emissiveFactor{ 0.f };
			float normalScale{ 1.f };
			float occlusionStrenth{ 1.f };
			glm::vec4 baseColorFactor{ 1.0f };
			float metallicFactor{ 1.f };
			float roughnessFactor{ 1.f };
			std::set<std::string> vCompileDefinitions;
			bool doubleSided{ false };

			// KHR_materials_clearcoat
			float clearcoatFactor{ 0.f };
			float clearcoatRoughnessFactor{ 0.f };

			// KHR_materials_emissive_strength
			float emissiveStrength{ 1.f };

			// KHR_materials_ior
			float ior{ 1.5f };

			// KHR_materials_iridescence
			float iridescenceFactor{ 0.f };
			float iridescenceIor{ 1.3f };
			float iridescenceThicknessMinimum{ 100.f };
			float iridescenceThicknessMaximum{ 400.f };

			// KHR_materials_sheen
			glm::vec3 sheenColorFactor{ 0.f };
			float sheenRoughnessFactor{ 0.f };

			// KHR_materials_specular
			float specularFactor{ 1.f };
			glm::vec3 specularColorFactor{ 1.f };


			// KHR_materials_transmission
			float transmissionFactor{ 0.f };

			// KHR_materials_unlit ?

			// KHR_materials_volume
			float thicknessFactor{ 0.f };
			float attenuationDistance{ INFINITY };
			glm::vec3 attenuationColor{ 1.f };
		};

		struct FMaterialCreateInfo
		{
			std::string baseShaderName{};
			std::set<std::string> definitions{};
			std::unordered_map<std::string, size_t> textures{};
			size_t usageCount{ 0ull };

			FMaterial parameters{};
		};

		struct FVertexAttributePointer
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			uint8_t* data{ nullptr };
			size_t stride{ 0ull };
			size_t count{ 0ull };
		};

		class CUniformHandle
		{
		public:
			template<class _Ty>
			void create(const _Ty& block)
			{
				m_data.resize(block.size);
				m_uniforms = block.uniforms;
			}

			// For render script
			void set(const char* name, const void* raw_data, size_t size)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end() || !raw_data)
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = uniform.size;

				std::memcpy(m_data.data() + uniform.offset, raw_data, size);
			}

			template<class _Ty>
			void set(const char* name, const _Ty& object, size_t size = 0ull)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				std::memcpy(m_data.data() + uniform.offset, &object + uniform.offset, size);
			}

			template<class _Ty>
			_Ty* get(const char* name)
			{
				auto found = m_uniforms.find(utl::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return nullptr;

				auto& uniform = found->second;

				if (sizeof(_Ty) > uniform.size)
					return nullptr;

				return reinterpret_cast<_Ty*>(m_data.data() + uniform.offset);
			}

			const std::vector<char>& data() const
			{
				return m_data;
			}
		private:
			std::unordered_map<size_t, FReflectionUniform> m_uniforms{};
			std::vector<char> m_data{};
		};

		class IShaderObject
		{
		public:
			virtual ~IShaderObject() = default;

			virtual void create(const std::string& name, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual void create(const std::string& name, const FComputeShaderCreateInfo& create_info) = 0;

			virtual const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) = 0;
			virtual const std::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) = 0;

			virtual void bindTexture(const char* name, size_t texture_resource) = 0;

			virtual void flushDescriptors() = 0;
			virtual void flushConstants(uint32_t queue) = 0;
			virtual void bind(uint32_t queue) = 0;
		};

		class IRenderMaterial
		{
		public:
			virtual ~IRenderMaterial() = default;

			virtual void addTexture(const char* name, size_t texture_id) = 0;
			virtual void bind() = 0;
		};

		class IRenderingStep
		{
		public:
			virtual ~IRenderingStep() = default;

			virtual void create(const FRenderingStepCreateInfo& create_info) = 0;
			virtual void execute(float dt) = 0;
		};

		class IVertexHandler
		{
		public:
			virtual ~IVertexHandler() = default;

			virtual void create() = 0;

			virtual size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) = 0;
			virtual size_t addIndices(const FVertexAttributePointer& indices) = 0;

			virtual size_t getLastVertex() = 0;
			virtual size_t getLastIndex() = 0;
		};

		class IRenderInterface
		{
		public:
			virtual ~IRenderInterface() = default;

			virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
			virtual void render(float dt) = 0;

			virtual size_t createTexture(const std::string& path) = 0;

			virtual size_t createShader(const std::string& name_info, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual size_t createShader(const std::string& name_info, const FComputeShaderCreateInfo& create_info) = 0;

			virtual size_t createMaterial(const std::string& name, const FMaterialCreateInfo& material_ci) = 0;

			virtual FShaderUploadInfo getShaderNameTag(const std::string& name) = 0;
			virtual void createRendering(const FRenderingStepCreateInfo& rendering_ci) = 0;

			virtual void updateSurfaceSize(int32_t width, int32_t height) = 0;
		};

		class IRenderContext
		{
		public:
			virtual ~IRenderContext() = default;

			virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
			virtual size_t createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) = 0;
			virtual size_t createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;

			virtual size_t createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual size_t createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;

			virtual void beginRendering(const FBeginRenderingInfo& rendering_info) = 0;
			virtual void endRendering(const FBeginRenderingInfo& rendering_info) = 0;

			virtual void setViewport(float width, float height, bool flip_viewport) = 0;
			virtual void setScissorRect(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;

			virtual void bindShader(size_t shader_id, uint32_t queue_type) = 0;

			virtual FShaderReflectionData getShaderReflection(const size_t& shader_view) = 0;
			virtual void updateShaderResources(const FShaderResourceCommit& resources, uint32_t /*EShaderResourcesUpdateFlagBits*/ flags = SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT) = 0;
			virtual void flushConstantRanges(size_t shader_id, const std::unordered_map<size_t, FUniformDataView>& constants, uint32_t queue_type) = 0;

			virtual size_t getTextureResource(const char* name) = 0;
			virtual size_t getShaderResource(const char* name) = 0;

			virtual FPoint2D<uint32_t> getRenderResolution() = 0;

			virtual uint32_t getDepthFormat() = 0;
			virtual uint32_t getApiFormat(uint32_t global_format) = 0;
			virtual uint32_t getPresentFormat() = 0;

			virtual void updateSurface(int32_t surface_width, int32_t surface_height) = 0;

			virtual void beginFrameCommandBuffers() = 0;
			virtual void endFrameCommandBuffers() = 0;
		};
	}
}
#pragma once

#include <engine/interface/SharedStructure.h>
#include "BufferStructures.h"
#include "LightStructures.h"
#include "MaterialStructures.h"
#include "RenderingStructures.h"
#include "ShaderStructures.h"
#include "TextureStructures.h"
#include "VertexStructures.h"
#include "RenderGraph.h"
#include "PostProcessStructures.h"

#include <container/hash_map.h>

#include <typeindex>

namespace engine
{
	namespace render
	{
		class IRenderContext;
		class IRenderInterface;

#ifndef BUILD_WITHOUT_DEBUGGIND
		namespace debug_mapping
		{
			inline constexpr std::pair<u64, const char*> uniform_to_name_mapping[] =
			{
				{ utl::utils::fnv1a_64_hash("GlobalUniformFrameData"), "GlobalUniformFrameData" },
				{ utl::utils::fnv1a_64_hash("UniformMeshData"), "UniformMeshData" },
				{ utl::utils::fnv1a_64_hash("UniformSkinningJoints"), "UniformSkinningJoints" },
			};

			constexpr const char* find(u64 key) 
			{
				for (const auto& kv : uniform_to_name_mapping)
				{
					if (kv.first == key)
						return kv.second;
				}

				return nullptr;
			}
		}
#endif

		enum class ERenderBufferIndices : u32
		{
			// Default geometry buffers
			eNodeBuffer = 0u,
			eSkinBuffer,

			eMaterialBuffer,
			// Light source buffers
			eDirectionalLightBuffer,
			ePointLightBuffer,
			eSpotLightBuffer,
			// Shadow buffers
			eCascadedShadowBuffer,
			eOmniShadowBuffer,
			eSpotShadowBuffer,
			// Culling and draw buffers
			eMeshBuffer,
			eDrawMeshInfoBuffer,
			eFrustumBuffer,
			eDrawCommandBuffer,
			eMeshCommandBuffer,
			eClusterInfoBuffer,
			// Other
			eTextureRemapBuffer,
			eViewportBuffer,

			eLastIndex
		};

		struct FGlobalUniformCreateInfo
		{
			utl::hash_map<u64, u64> uniforms{};
		};

		struct FGlobalUniformUpdateInfo
		{
			utl::hash_map<u64, FUniformDataView> uniforms{};
		};

		struct FRenderCamera
		{
			f32 fov{ 45.f };
			f32 nearp{ 0.1f };
			f32 farp{ 512.f };

			glm::mat4 view{ 1.f };
			glm::mat4 invView{ 1.f };

			glm::vec3 up{ 0.f };
			glm::vec3 forward{ 0.f };
			glm::vec3 right{ 0.f };

			glm::vec2 dimensions{};

			FFrustum frustum{};
		};

		struct FRenderNode
		{
			glm::mat4 model{};
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
			void set(const char* name, const void* raw_data, u64 size)
			{
				auto found = m_uniforms.find(utl::utils::fnv1a_64_hash(name));
				if (found == m_uniforms.end() || !raw_data)
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = uniform.size;

				std::memcpy(m_data.data() + uniform.offset, raw_data, size);
			}

			template<class _Ty>
			void set(const char* name, const _Ty& object, u64 size = 0ull)
			{
				auto found = m_uniforms.find(utl::utils::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				std::memcpy(m_data.data() + uniform.offset, &object, size);
			}

			template<class _Ty>
			_Ty* get(const char* name)
			{
				auto found = m_uniforms.find(utl::utils::fnv1a_64_hash(name));
				if (found == m_uniforms.end())
					return nullptr;

				auto& uniform = found->second;

				if (sizeof(_Ty) > uniform.size)
					return nullptr;

				return reinterpret_cast<_Ty*>(m_data.data() + uniform.offset);
			}

			const utl::vector<char>& data() const
			{
				return m_data;
			}
		private:
			utl::hash_map<u64, FReflectionUniform> m_uniforms{};
			utl::vector<char> m_data{};
		};

		class IShaderObject
		{
		public:
			virtual ~IShaderObject() = default;

			virtual void create(const std::string& name, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual void create(const std::string& name, const FComputeShaderCreateInfo& create_info) = 0;

			virtual const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name) = 0;
			virtual const std::unique_ptr<CUniformHandle>& getConstantBlock(const char* name) = 0;

			virtual void bindBuffer(const char* name, u64 buffer_resource) = 0;
			virtual void bindTexture(const char* name, u64 texture_resource, u32 mip = ~0u) = 0;

			virtual void flushDescriptors() = 0;
			virtual void flushConstants(EQueueTypeFlagBits queue) = 0;
			virtual void bind(EQueueTypeFlagBits queue) = 0;

			virtual u64 handle() const = 0;

			virtual const FShaderReflectionData& getReflection() const = 0;
		};

		struct FRenderSceneStats
		{
			u64 vboCount{ 0ull };
			u64 primitiveCount{ 0ull };
			u64 materialCount{ 0ull };
			u64 nodeCount{ 0ull };

			// Type is a hash of all used attributes
			utl::hash_map<u64, u64> perTypeVBOCount{};
		};

		struct FCullingInfo
		{
			u64 cullerID{};
			ERenderGeometryCullerType type{};
		};

		struct FRenderDataPacket
		{
			FRenderDataPacket()
			{
				//resources = std::make_unique<CGPUArrayHolder>();
			}

			FRenderCamera camera{};
			FRenderSceneStats stats{};
		};

		struct FRenderWorldStats
		{
			u64 directionalLightCount{ 0ull };
			u64 pointLightCount{ 0ull };
			u64 spotLightCount{ 0ull };

			u64 cascadedShadowCount{ 0ull };
			u64 omniShadowCount{ 0ull };
			u64 spotShadowCount{ 0ull };

			u64 meshCount{ 0ull };
			u64 skinCount{ 0ull };

			u64 drawCommandCount{ 0ull };
		};

		struct FDeviceLimitations
		{
			u64 maxBatchSize{ 1ull };
		};

		struct FRenderFrameStats
		{
			utl::vector<f32> timestampResults{};
			f32 gpuFrameTime{};

			u64 totalGPUMemory{};
			u64 usedGPUMemory{};
		};

		class IRenderContext
		{
		public:
			virtual ~IRenderContext() = default;

			virtual void create(const FRenderContextCreateInfo& context_ci) = 0;
			virtual void createGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) = 0;
			virtual void updateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) = 0;
			virtual const utl::vector<uint32_t>& getGlobalTextureRemap() = 0;

			virtual u64 createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info) = 0;
			virtual u64 getTextureId(const std::string& name) = 0;
			virtual u64 createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual const FTextureCreateInfo& getTextureInfo(u64 texture_id) = 0;
			virtual void destroyTexture(u64 resource_id) = 0;

			virtual u64 createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual u64 createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual u64 createShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual FShaderReflectionData getShaderReflection(const u64& shader_view) = 0;
			virtual void updateShaderResources(const FShaderResourceCommit& resources, EShaderResourcesUpdateFlags flags = EShaderResourcesUpdateFlagBits::eAll) = 0;
			virtual void flushConstantRanges(u64 shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) = 0;
			virtual void bindShader(u64 shader_id, EQueueTypeFlagBits queue_type) = 0;
			virtual void destroyShader(u64 resource_id) = 0;

			virtual u64 createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void updateBuffer(const std::string& name, u64 buffer_id, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void uploadDataToBuffer(const FBufferUploadInfo& upload_info) = 0;
			virtual FBufferView getBufferView(u64 buffer_id) = 0;
			virtual void bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void mapBufferMemory(u64 buffer_id, void*& memory) = 0;
			virtual void unmapBufferMemory(u64 buffer_id) = 0;
			virtual void flushBuffer(u64 buffer_id, u64 size, u64 offset) = 0;
			virtual void destroyBuffer(u64 resource_id) = 0;

			virtual void pushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<u64>& textures) = 0;
			virtual void pushC2CBarrier(EQueueTypeFlagBits queue_type) = 0;
			virtual void pushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids) = 0;

			virtual void processResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers) = 0;

			virtual void beginTimestampZone(EQueueTypeFlagBits queue) = 0;
			virtual void endTimestampZone(EQueueTypeFlagBits queue) = 0;

			virtual void beginRendering(const FBeginRendering2Info& rendering_info) = 0;
			virtual void endRendering(EQueueTypeFlagBits queue) = 0;

			virtual void beginCompute(const FBeginComputeInfo& compute_info) = 0;
			virtual void endCompute(EQueueTypeFlagBits queue) = 0;

			virtual void setViewport(f32 width, f32 height, bool flip_viewport) = 0;
			virtual void setScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) = 0;

			virtual void draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) = 0;
			virtual void draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) = 0;
			virtual void dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group, u64 subgroup_index) = 0;
			virtual void dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) = 0;

			virtual FDeviceLimitations getDeviceLimitations() = 0;
			virtual const FRenderFrameStats& getRenderFrameStats() const = 0;

			virtual u64 getTextureResource(const char* name) = 0;
			virtual u64 getShaderResource(const char* name) = 0;

			virtual FPoint2D<u32> getRenderResolution() const = 0;
			virtual FRect2D<f32> getRenderRectFromRT(u64 texture_id) = 0;
			virtual FRect2D<f32> getScreenRenderRect() = 0;

			virtual u32 getDepthFormat(bool depth_only) = 0;
			virtual u32 getApiFormat(EApiDataFormat global_format) = 0;
			virtual u32 getPresentFormat() = 0;

			virtual void updateSurface(i32 surface_width, i32 surface_height) = 0;

			virtual void beginFrame() = 0;
			virtual void endFrame() = 0;

			virtual bool isSwapChainFinished() = 0;

			virtual void beginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) = 0;
			virtual void insertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) = 0;
			virtual void endDebugLabel(EQueueTypeFlagBits queue_type) = 0;
		};

		class IRenderWorld
		{
		public:
			virtual ~IRenderWorld() = default;

			virtual void create() = 0;

			virtual u64 registerFrustum(ERenderGeometryCullerType type) = 0;
			virtual void updateFrustum(u64 frustum_id, const glm::mat4& view, const glm::mat4& projection) = 0;

			virtual void addDrawGeometry(const utl::vector<FGeometryRawBatch>& new_geometry) = 0;

			virtual void render(f32 dt) = 0;

			virtual void addLight(const FDirectionalLightCommit& light) = 0;
			virtual void addLight(const FPointLightCommit& light) = 0;
			virtual void addLight(const FSpotLightCommit& light) = 0;

			virtual void addShadow(const FCascadedShadowCommit& commit) = 0;
			virtual void addShadow(const FOmniShadowCommit& commit) = 0;
			virtual void addShadow(const FSpotShadowCommit& commit) = 0;

			virtual void updateNode(u32 node_id, const FSceneNode& node) = 0;
			virtual void updateSkin(const utl::vector<glm::mat4>& matrices) = 0;

			virtual void addMaterial(const FMaterial& material) = 0;
			virtual void addMaterials(const utl::vector<FMaterial>& materials) = 0;
			virtual const FMaterial& getMaterial(u64 index) const = 0;

			virtual void addMesh(const FMesh& mesh) = 0;
			virtual void addMeshes(const utl::vector<FMesh>& meshes) = 0;

			virtual const std::unique_ptr<IGPUVector>& getDrawBuffer(ERenderBufferIndices buffer_index) = 0;
			//virtual const utl::vector<std::unique_ptr<IBufferObject>>& getDrawBuffers() = 0;

			virtual const utl::vector<FGeometryCPUBatch>& getGeometryBatches() const = 0;

			virtual FPostProcess& getPostProcessingData() = 0;

			virtual FRenderWorldStats& getStats() = 0;
			virtual const FRenderWorldStats& getStats() const = 0;
		};

		class IRenderInterface
		{
		public:
			virtual ~IRenderInterface() = default;

			virtual void create(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
			virtual void render(f32 dt) = 0;

			virtual u64 createTexture(const std::string& path) = 0;
			virtual u64 loadTexture(const std::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info) = 0;

			virtual u64 createShader(const std::string& name_info, const FGraphicsShaderCreateInfo& create_info) = 0;
			virtual u64 createShader(const std::string& name_info, const FComputeShaderCreateInfo& create_info) = 0;

			virtual u64 createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) = 0;
			virtual const std::unique_ptr<IVertexHandler>& getVertexHandler(u64 id) = 0;

			virtual FShaderUploadInfo getShaderNameTag(const std::string& name) = 0;

			virtual void updateSurfaceSize(i32 width, i32 height) = 0;

			virtual const std::unique_ptr<FRenderDataPacket>& getRenderPacket() = 0;
			virtual const std::unique_ptr<IRenderGraph>& getRenderGraph() = 0;
			virtual const std::unique_ptr<IRenderWorld>& getRenderWorld() = 0;
			virtual const std::unique_ptr<IRenderContext>& getRenderContext() = 0;
		};
	}
}
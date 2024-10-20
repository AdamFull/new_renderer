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
	namespace filesystem { class IVirtualFileSystemInterface; }

	namespace render
	{
		class IRenderBackend;
		class IRenderInterface;

		namespace shader_bindings
		{
			using namespace utl;
			constexpr const u64 global_uniform_frame_data = "GlobalUniformFrameData"_hash64;
			constexpr const u64 global_buffer_references = "GlobalBufferReferences"_hash64;
			constexpr const u64 global_post_process_data = "GlobalPostProcessingData"_hash64;
			constexpr const u64 uniform_mesh_data = "UniformMeshData"_hash64;
			constexpr const u64 uniform_skinning_joints = "UniformSkinningJoints"_hash64;
			constexpr const u64 external_bind_resources = "FExternalBindResources"_hash64;
			constexpr const u64 histogram_compute_params = "FHistogramComputeParams"_hash64;
			constexpr const u64 min_log_lum_label = "minLogLum"_hash64;
			constexpr const u64 log_lum_range_label = "logLumRange"_hash64;
			constexpr const u64 time_coeff_label = "timeCoeff"_hash64;
			constexpr const u64 time_num_pixels_x_label = "numPixelsX"_hash64;
			constexpr const u64 time_num_pixels_y_label = "numPixelsY"_hash64;
			constexpr const u64 references = "references"_hash64;
			constexpr const u64 frustum_label = "frustum"_hash64;
			constexpr const u64 shader_data = "shadowData"_hash64;
			constexpr const u64 call_id_label = "callID"_hash64;
			constexpr const u64 shadow_index_label = "shadowIndex"_hash64;
			constexpr const u64 layer_label = "layer"_hash64;
			constexpr const u64 compute_cubemap_data = "ComputeCubemapData"_hash64;
			constexpr const u64 roughness_label = "roughness"_hash64;
			constexpr const u64 mesh_data = "meshData"_hash64;
			constexpr const u64 batch_id_label = "batchID"_hash64;
			constexpr const u64 vbo_id_label = "vboId"_hash64;
			constexpr const u64 geometry_buffers = "GeometryBuffers"_hash64;
			constexpr const u64 vb_label = "vb"_hash64;
			constexpr const u64 ib_label = "ib"_hash64;
			constexpr const u64 mb_label = "mb"_hash64;
			constexpr const u64 vrb_label = "vrb"_hash64;
			constexpr const u64 bb_label = "bb"_hash64;
			constexpr const u64 tb_label = "tb"_hash64;
			constexpr const u64 lb_label = "lb"_hash64;
			constexpr const u64 last_index_label = "lastIndex"_hash64;
			constexpr const u64 last_vertex_label = "lastVertex"_hash64;
		}
			
#ifndef BUILD_WITHOUT_DEBUGGIND
		namespace debug_mapping
		{
			inline constexpr std::pair<u64, const char*> uniform_to_name_mapping[] =
			{
				{ shader_bindings::global_uniform_frame_data, "GlobalUniformFrameData" },
				{ shader_bindings::global_buffer_references, "GlobalBufferReferences" },
				{ shader_bindings::global_post_process_data, "GlobalPostProcessingData" },
				{ shader_bindings::uniform_mesh_data, "UniformMeshData" },
				{ shader_bindings::uniform_skinning_joints, "UniformSkinningJoints" },
				{ shader_bindings::external_bind_resources, "FExternalBindResources" },
				{ shader_bindings::histogram_compute_params, "FHistogramComputeParams" },
				{ shader_bindings::min_log_lum_label, "minLogLum" },
				{ shader_bindings::log_lum_range_label, "logLumRange" },
				{ shader_bindings::time_coeff_label, "timeCoeff" },
				{ shader_bindings::time_num_pixels_x_label, "numPixelsX" },
				{ shader_bindings::time_num_pixels_y_label, "numPixelsY" },
				{ shader_bindings::references, "references" },
				{ shader_bindings::frustum_label, "frustum" },
				{ shader_bindings::shader_data, "shadowData" },
				{ shader_bindings::call_id_label, "callID" },
				{ shader_bindings::shadow_index_label, "shadowIndex" },
				{ shader_bindings::layer_label, "layer" },
				{ shader_bindings::compute_cubemap_data, "ComputeCubemapData" },
				{ shader_bindings::roughness_label, "roughness" },
				{ shader_bindings::mesh_data, "meshData" },
				{ shader_bindings::batch_id_label, "batchID" },
				{ shader_bindings::vbo_id_label, "vboId" },
				{ shader_bindings::geometry_buffers, "GeometryBuffers" },
				{ shader_bindings::vb_label , "vb" },
				{ shader_bindings::ib_label , "ib" },
				{ shader_bindings::mb_label , "mb" },
				{ shader_bindings::vrb_label, "vrb" },
				{ shader_bindings::bb_label , "bb" },
				{ shader_bindings::tb_label , "tb" },
				{ shader_bindings::lb_label , "lb" },
				{ shader_bindings::last_index_label , "lastIndex" },
				{ shader_bindings::last_vertex_label , "lastVertex" },
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
			eMatricesBuffer,

			eMaterialBuffer,
			// Light source buffers
			eLightBuffer,
			// Shadow buffers
			eCascadedShadowBuffer,
			eOmniShadowBuffer,
			eSpotShadowBuffer,
			// Culling and Draw buffers
			eMeshBuffer,
			eDrawMeshInfoBuffer,
			eRawDrawMeshInfoBuffer,
			eFrustumBuffer,
			eDrawCommandBuffer,
			eCountBuffer,
			eMeshCommandBuffer,
			eClusterInfoBuffer,
			// Other
			eTextureRemapBuffer,
			eCameraBuffer,

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

		struct FRenderNode
		{
			glm::mat4 model{};
		};

		class CUniformHandle
		{
		public:
			template<class _Ty>
			void Create(const _Ty& block)
			{
				m_data.resize(block.size);
				m_uniforms = block.uniforms;
			}

			// For render script
			void Set(u64 name, const void* raw_data, u64 size)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end() || !raw_data)
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = uniform.size;

				util::Copy(m_data.data(), uniform.offset, raw_data, size);
			}

			template<class _Ty>
			void Set(u64 name, const _Ty& object, u64 size = 0ull)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				util::Copy(m_data.data(), uniform.offset, &object, size);
			}

			template<class _Ty>
			_Ty* Get(u64 name)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end())
					return nullptr;

				auto& uniform = found->second;

				if (sizeof(_Ty) > uniform.size)
					return nullptr;

				return reinterpret_cast<_Ty*>(m_data.data() + uniform.offset);
			}

			const utl::vector<char>& Data() const
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

			virtual void Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info, const std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual void Create(const std::string& name, const FComputeShaderCreateInfo& create_info, const FShaderCreationBlockInfo& shader_stage) = 0;

			virtual const std::unique_ptr<CUniformHandle>& GetUniformBlock(u64 name) = 0;
			virtual const std::unique_ptr<CUniformHandle>& GetConstantBlock(u64 name) = 0;

			virtual void BindBuffer(u64 name, FAPIHandle buffer_resource) = 0;
			virtual void BindTexture(u64 name, FAPIHandle texture_resource, u32 mip = ~0u) = 0;

			virtual void FlushConstants(EQueueTypeFlagBits queue) = 0;
			virtual void Bind(EQueueTypeFlagBits queue) = 0;
			virtual void BindResources(EQueueTypeFlagBits queue) = 0;

			virtual FAPIHandle Handle() const = 0;

			virtual const FShaderReflectionData& GetReflection() const = 0;
		};

		struct FRenderSceneStats
		{
			u64 vboCount{ 0ull };
			u64 primitiveCount{ 0ull };
			u64 materialCount{ 0ull };
			u64 nodeCount{ 0ull };
		};

		struct FCullingInfo
		{
			u64 cullerID{};
			ERenderGeometryCullerType type{};
		};

		struct FDrawInfo
		{
			u64 first{ 0ull };
			u64 count{ 0ull };
			u64 drawCount{ 0ull };
			u64 cullerID{ 0ull };
		};

		struct FRenderDataPacket
		{
			FRenderDataPacket()
			{
				//resources = std::make_unique<CGPUArrayHolder>();
			}

			FRenderSceneStats stats{};
		};

		struct FRenderWorldStats
		{
			u64 directionalLightCount{ 0ull };
			u64 pointLightCount{ 0ull };
			u64 spotLightCount{ 0ull };
			u64 lightCount{ 0ull };

			u64 cascadedShadowCount{ 0ull };
			u64 omniShadowCount{ 0ull };
			u64 spotShadowCount{ 0ull };

			u64 meshCount{ 0ull };
			u64 matricesCount{ 0ull };

			u64 cullerCount{ 0ull };
			u64 maxDrawCommands{ 0ull };
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

		class IRenderBackend
		{
		public:
			virtual ~IRenderBackend() = default;

			virtual void Create(const FRenderContextCreateInfo& context_ci) = 0;
			virtual void CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci) = 0;
			virtual void UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view) = 0;
			virtual const utl::vector<uint32_t>& GetGlobalTextureRemap() = 0;

			virtual FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual FAPIHandle CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info) = 0;
			virtual FAPIHandle GetTextureId(const std::string& name) = 0;
			virtual FTextureView GetTextureView(FAPIHandle texture_id) = 0;
			virtual void DestroyTexture(FAPIHandle resource_id) = 0;
			virtual void ReadPixel(FAPIHandle resource_id, u32 x, u32 y, void* data) = 0;

			virtual FAPIHandle CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const std::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual FAPIHandle CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const FShaderCreationBlockInfo& shader_stage) = 0;
			virtual FAPIHandle CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci) = 0;
			virtual FShaderReflectionData GetShaderReflection(FAPIHandle shader_view) = 0;
			virtual void UpdateUniform(FAPIHandle shader_id, uint64_t reflection_name, std::span<const char> data) = 0;
			virtual void BindTexture(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle texture_id, uint32_t mip_level) = 0;
			virtual void BindBuffer(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle buffer_id) = 0;
			virtual void FlushConstantRanges(FAPIHandle shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type) = 0;
			virtual void BindShader(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) = 0;
			virtual void ResetShaderResources(FAPIHandle shader_id) = 0;
			virtual void BindShaderResources(FAPIHandle shader_id, EQueueTypeFlagBits queue_type) = 0;
			virtual void DestroyShader(FAPIHandle resource_id) = 0;

			virtual FAPIHandle CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void UpdateBuffer(const std::string& name, FAPIHandle buffer_id, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void BeginUpdateBuffer(BufferUpdateInfo& update_info) = 0;
			virtual void EndUpdateBuffer(BufferUpdateInfo& update_info) = 0;
			virtual FBufferView GetBufferView(FAPIHandle buffer_id) = 0;
			virtual void BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue) = 0;
			virtual void MapBufferMemory(FAPIHandle buffer_id, void*& memory) = 0;
			virtual void UnmapBufferMemory(FAPIHandle buffer_id) = 0;
			virtual void FlushBuffer(FAPIHandle buffer_id, u64 size, u64 offset) = 0;
			virtual void DestroyBuffer(FAPIHandle resource_id) = 0;

			virtual void ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers) = 0;

			virtual void BeginTimestampZone(EQueueTypeFlagBits queue) = 0;
			virtual void EndTimestampZone(EQueueTypeFlagBits queue) = 0;

			virtual void BeginRendering(const FBeginRendering2Info& rendering_info) = 0;
			virtual void EndRendering(EQueueTypeFlagBits queue) = 0;

			virtual void BeginCompute(const FBeginComputeInfo& compute_info) = 0;
			virtual void EndCompute(EQueueTypeFlagBits queue) = 0;

			virtual void SetViewport(f32 width, f32 height, bool flip_viewport) = 0;
			virtual void SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height) = 0;
			virtual void SetPrimitiveTopology(EVertexPrimitiveTopology primitivd_topology) = 0;

			virtual void Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info) = 0;
			virtual void Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info) = 0;
			virtual void Dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_group, u64 subgroup_index) = 0;
			virtual void Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info) = 0;

			virtual FDeviceLimitations GetDeviceLimitations() = 0;
			virtual const FRenderFrameStats& GetRenderFrameStats() const = 0;
			virtual EAdapterVendor GetDeviceVendor() const = 0;

			virtual FAPIHandle GetTextureResource(const char* name) = 0;
			virtual FAPIHandle GetShaderResource(const char* name) = 0;

			virtual FPoint2D<u32> GetRenderResolution() const = 0;
			virtual FRect2D<f32> GetRenderRectFromRT(FAPIHandle texture_id) = 0;
			virtual FRect2D<f32> GetScreenRenderRect() = 0;

			virtual void BeginFrame() = 0;
			virtual void EndFrame() = 0;

			virtual bool IsSwapChainFinished() = 0;

			virtual void BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) = 0;
			virtual void InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color = 0xffffffff) = 0;
			virtual void EndDebugLabel(EQueueTypeFlagBits queue_type) = 0;
		};

		class IRenderWorld
		{
		public:
			virtual ~IRenderWorld() = default;

			virtual void Create() = 0;

			virtual u64 RegisterCamera(ERenderGeometryCullerType type) = 0;
			virtual void UpdateCamera(u64 camera_id, const glm::mat4& view, const glm::mat4& prev_view, const glm::mat4& projection, const glm::mat4& prev_projection, const glm::vec2& size, float znear = 0.f, float zfar = 1.f) = 0;
			virtual const FCamera& GetCamera(u64 index) const = 0;
			virtual const FCamera& GetDefaultCamera() const = 0;
			virtual const u64 GetCameraID(ERenderGeometryCullerType type) const = 0;
			virtual const u64 GetDefaultCameraID() const = 0;

			virtual void AddDrawGeometry(const utl::vector<FGeometryBatch>& new_geometry) = 0;

			virtual void Begin(f32 dt) = 0;
			virtual void End() = 0;

			virtual u64 UpdateLight(const FLightCommit& light) = 0;

			virtual void AddShadow(const FCascadedShadowCommit& commit) = 0;
			virtual void AddShadow(const FOmniShadowCommit& commit) = 0;
			virtual void AddShadow(const FSpotShadowCommit& commit) = 0;

			virtual void AddMatrices(const utl::vector<glm::mat4>& matrices) = 0;
			virtual const glm::mat4& GetMatrix(u64 index) const = 0;

			virtual u64 UpdateNode(u32 node_id, const FSceneNode& node) = 0;

			virtual void AddMaterial(const FMaterial& material) = 0;
			virtual void AddMaterials(const utl::vector<FMaterial>& materials) = 0;
			virtual const FMaterial& GetMaterial(u64 index) const = 0;

			virtual void AddMesh(const FMesh& mesh) = 0;
			virtual void AddMeshes(const utl::vector<FMesh>& meshes) = 0;

			virtual const std::unique_ptr<IGPUVector>& GetDrawBuffer(ERenderBufferIndices buffer_index) const = 0;
			//virtual const utl::vector<std::unique_ptr<IBufferObject>>& getDrawBuffers() = 0;

			virtual const utl::vector<FGeometryCPUBatch>& GetGeometryBatches() const = 0;
			virtual const utl::vector<FDrawInfo>& GetDrawCommandByType(ERenderGeometryCullerType type) const = 0;

			virtual FPostProcess& GetPostProcessingData() = 0;

			virtual FRenderWorldStats& GetStats() = 0;
			virtual const FRenderWorldStats& GetStats() const = 0;
		};

		class IResourceLoader
		{
		public:
			virtual ~IResourceLoader() = default;

			virtual void Initialize(filesystem::IVirtualFileSystemInterface* vfs) = 0;

			virtual bool LoadTexture(const utl::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle) = 0;
			virtual bool LoadShaderData(const FShaderUploadInfo& load_info, utl::vector<FShaderCreationBlockInfo>& shader_stages) = 0;
			virtual void DestroyHandle(FTextureResourceHandle& handle) = 0;
		};

		class IRenderInterface
		{
		public:
			virtual ~IRenderInterface() = default;

			virtual void PreInitialize(const FRenderInterfaceCreateInfo& render_interface_ci) = 0;
			virtual void Initialize() = 0;
			virtual void PostInitialize() = 0;

			virtual void PreUpdate(f32 dt) = 0;
			virtual void Update(f32 dt) = 0;
			virtual void PostUpdate(f32 dt) = 0;

			virtual void PreRender() = 0;
			virtual void Render() = 0;
			virtual void PostRender() = 0;

			virtual FRHIHandle CreateTextureObject(const std::string& path) = 0;
			virtual FRHIHandle CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci) = 0;
			virtual FRHIHandle CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info) = 0;
			virtual void DestroyTextureObject(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<ITextureObject>& GetTextureObject(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<ITextureObject>& GetTextureObject(const char* name) = 0;

			virtual FRHIHandle CreateBufferObject(const std::string& name, const FBufferCreateInfo& buffer_ci) = 0;
			virtual void DestroyBufferObject(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<IBufferObject>& GetBufferObject(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<IBufferObject>& GetBufferObject(const char* name) = 0;

			virtual FRHIHandle CreateShaderObject(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci) = 0;
			virtual FRHIHandle CreateShaderObject(const std::string& name, const FComputeShaderCreateInfo& shader_ci) = 0;
			virtual void DestroyShaderObject(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<IShaderObject>& GetShaderObject(FRHIHandle id) = 0;
			virtual const std::unique_ptr<IShaderObject>& GetShaderObject(const char* name) = 0;

			virtual FRHIHandle CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci) = 0;
			virtual void DestroyVertexHandler(FRHIHandle handle) = 0;
			virtual const std::unique_ptr<IVertexHandler>& GetVertexHandler(FRHIHandle id) = 0;

			virtual FShaderUploadInfo GetShaderNameTag(const std::string& name) = 0;

			virtual void UpdateSurfaceSize(i32 width, i32 height) = 0;

			virtual const std::unique_ptr<FRenderDataPacket>& GetRenderPacket() = 0;
			virtual const std::unique_ptr<IRenderGraph>& GetRenderGraph() = 0;
			virtual const std::unique_ptr<IRenderWorld>& GetRenderWorld() = 0;
			virtual const std::unique_ptr<IRenderBackend>& GetRenderBackend() = 0;
			virtual const std::unique_ptr<IResourceLoader>& GetResourceLoader() = 0;
		};
	}
}
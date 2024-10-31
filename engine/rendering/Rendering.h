#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/RenderResourceContainer.h>

#include <engine/rendering/ExecutionContext.h>

//#include <engine/rendering/GPUVector.h>
#include <engine/rendering/render_graph/ShaderPass.h>
#include <engine/rendering/render_graph/RenderGraph.h>
#include <memory>

namespace engine
{
	class Window;

	namespace gfx
	{
		class Context;
		class QueryPool;
		class Sampler;
		class ResourceContext;
	}

	namespace render
	{
		class RenderContext;

		class Renderer : public IRenderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddRenderResource(const std::string& path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;

			uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) override;

			uint32_t AddShaderPass(const ComputeShaderPassCreateInfo& create_info);
			uint32_t AddShaderPass(const RenderShaderPassCreateInfo& create_info);

			void Render(float delta_time) override;

			gfx::Context& GetGFXContext();

			ExecutionContext& GetRenderContext();
			ExecutionContext const& GetRenderContext() const;

			gfx::ResourceContext& GetResourceContext();
			gfx::ResourceContext const& GetResourceContext() const;

			gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info) override;

			GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;
			GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;

			uint32_t AddRenderCamera() override;
			IRenderCamera& GetRenderCamera(uint32_t camera_id) override;

			IDynamicBuffer& GetNodeBuffer() override;
			IDynamicBuffer& GetMatrixBuffer() override;
			IDynamicBuffer& GetMeshBuffer() override;

			IDynamicBuffer& GetMaterialBuffer() override;
			IDynamicBuffer& GetLightBuffer() override;

			IDynamicBuffer& GetCascadedShadowBuffer() override;
			IDynamicBuffer& GetOmniShadowBuffer() override;
			IDynamicBuffer& GetSpotShadowBuffer() override;

			IDynamicBuffer& GetGeobetryBatchBuffer() override;
			IDynamicBuffer& GetRawGeobetryBatchBuffer() override;

			IDynamicBuffer& GetFrustumBuffer() override;

			IDynamicBuffer& GetIndirectDrawBuffer() override;
			IDynamicBuffer& GetIndirectCountDrawBuffer() override;
			IDynamicBuffer& GetMeshCommandBuffer() override;

			IDynamicBuffer& GetCameraBuffer() override;
		private:
			RenderGraphNodeHeader* CreateShaderPassNode(const ComputeShaderPassCreateInfo& create_info);
			RenderGraphNodeHeader* CreateShaderPassNode(const RenderShaderPassCreateInfo& create_info);

			RenderGraphNodeHeader* AllocateStaticConditionNode();
			RenderGraphNodeHeader* AllocateDynamicConditionNode();
			RenderGraphNodeHeader* AllocateGroupNode();

			gfx::Texture* GetTextureResource(const RenderGraphResource& graph_resource);

			void CollectRencerResouces(uint32_t pass_index, const ShaderPassCreateInfo& pass_ci);

			void DestroyRenderGraph();
			void DestoyRenderGraphNode(RenderGraphNodeHeader* node);

			void EvaluateShaderPass(uint32_t pass_index);

			void EvaluateRenderGraph(float delta_time);
			void EvaluateRenderGraphNode(RenderGraphNodeHeader* node, float delta_time);

			void BuildGraph();
			void BeginFrame();
			void EndFrame();
			void UpdateRenderTargets();
		private:
			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<gfx::ResourceContext> resource_context{};
			std::unique_ptr<RenderContext> render_context{};

			gfx::QueryPool* timestamp_query{ nullptr };
			float timestamp_frequency{ 0.f };
			float gpu_delta_time{ 0.f };

			utl::hash_map<uint32_t, gfx::Sampler*> sampler_cache{};
			gfx::Sampler* default_sampler{ nullptr };

			//GPUVector<FMaterial> materials{};

			gfx::SharedDescriptor* shared_descriptor{ nullptr };
			gfx::ResourceDescriptor* global_resource_descriptor{ nullptr };

			// Render object descriptions
			struct TextureObject
			{
				gfx::Texture* texture{ nullptr };
				gfx::Sampler* sampler{ nullptr };

				gfx::TextureUsageFlags usage{};
				bool resizable{ false };
			};
			ResourceContainer<TextureObject> textures{};
			ResourceContainer<TextureObject> render_targets{};

			struct BufferObject
			{
				gfx::Buffer* buffer{ nullptr };
			};
			ResourceContainer<BufferObject> buffers{};

			std::unordered_map<uint32_t, std::unique_ptr<GeometryBuffer>> interleaved_geometry_buffers{};
			std::unordered_map<uint32_t, std::unique_ptr<GeometryBuffer>> deinterleaved_geometry_buffers{};

			// Render graph resources
			std::vector<std::unique_ptr<ShaderPass>> shader_passes{};
			std::vector<ResourceBindings> per_pass_resource_bindings{};

			utl::hash_map<std::pair<uint32_t, uint32_t>, ConsistentAccessInfo, std_pair_hash> resource_access_map{};

			uint32_t backbuffer_resource{ ~0u };
			std::vector<RenderGraphResource> graph_resources{};

			std::vector<RenderGraphNodeHeader*> render_graph{};

			// Gpu scene
			std::vector<std::unique_ptr<IRenderCamera>> render_cameras{};

			std::unique_ptr<IDynamicBuffer> node_buffer{};
			std::unique_ptr<IDynamicBuffer> matrices_buffer{};
			std::unique_ptr<IDynamicBuffer> mesh_buffer{};

			std::unique_ptr<IDynamicBuffer> material_buffer{};
			std::unique_ptr<IDynamicBuffer> light_buffer{};

			std::unique_ptr<IDynamicBuffer> cascaded_shadow_buffer{};
			std::unique_ptr<IDynamicBuffer> omni_shadow_buffer{};
			std::unique_ptr<IDynamicBuffer> spot_shadow_buffer{};

			std::unique_ptr<IDynamicBuffer> geometry_batches_buffer{};
			std::unique_ptr<IDynamicBuffer> raw_geometry_batches_buffer{};

			std::unique_ptr<IDynamicBuffer> frustum_buffer{};

			std::unique_ptr<IDynamicBuffer> indexed_draw_indirect_buffer{};
			std::unique_ptr<IDynamicBuffer> indirect_count_buffer{};
			std::unique_ptr<IDynamicBuffer> mesh_command_buffer{};

			std::unique_ptr<IDynamicBuffer> camera_buffer{};
		};
	}
}
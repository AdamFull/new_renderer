#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/RenderResourceContainer.h>

#include <engine/rendering/ExecutionContext.h>

#include <engine/rendering/GPUVector.h>
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

			uint32_t AddTextureResource(const std::string& path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddTextureResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;

			uint32_t AddBufferResource(const gfx::BufferCreateInfo& create_info) override;

			uint32_t AddShaderPass(const ComputeShaderPassCreateInfo& create_info);
			uint32_t AddShaderPass(const RenderShaderPassCreateInfo& create_info);

			void Render(float delta_time) override;

			gfx::Context& GetGFXContext();

			ExecutionContext& GetRenderContext();
			ExecutionContext const& GetRenderContext() const;

			gfx::ResourceContext& GetResourceContext();
			gfx::ResourceContext const& GetResourceContext() const;

			gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info) override;
		private:
			RenderGraphNodeHeader* AllocateStaticConditionNode();
			RenderGraphNodeHeader* AllocateDynamicConditionNode();
			RenderGraphNodeHeader* AllocateGroupNode();
			RenderGraphNodeHeader* AllocatePassNode();

			gfx::TextureAccess GetFinalTextureLayout(const RenderGraphResource& graph_resource);
			gfx::TextureAccess GetIntermediateTextureLayout(const RenderGraphResource& graph_resource);

			gfx::TextureAccess GetCurrentTextureLayout(const RenderGraphResource& graph_resource);
			void UpdateTextureLayout(const RenderGraphResource& graph_resource, gfx::TextureAccess new_layout);

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

			// Render object descriptions
			struct TextureObject
			{
				gfx::Texture* texture{ nullptr };
				gfx::Sampler* sampler{ nullptr };

				gfx::TextureAccess current_access{ {}, gfx::TextureLayout::eUndefined };
			};
			ResourceContainer<TextureObject> textures{};
			ResourceContainer<TextureObject> render_targets{};

			struct BufferObject
			{
				gfx::Buffer* buffer{ nullptr };
				gfx::ResourceState resource_state{ gfx::AccessFlag::eNone, gfx::StageFlag::eAllCommands };
			};
			ResourceContainer<BufferObject> buffers{};

			// Render graph resources
			std::vector<std::unique_ptr<ShaderPass>> shader_passes{};
			std::vector<RenderGraphPassResourceAccess> per_pass_resouce_access{};
			std::vector<ResourceBindings> per_pass_resource_bindings{};

			utl::hash_map<std::pair<uint32_t, uint32_t>, ConsistentAccessInfo, std_pair_hash> resource_access_map{};

			gfx::TextureAccess backbuffer_layout{};
			uint32_t backbuffer_resource{ ~0u };
			std::vector<RenderGraphResource> graph_resources{};

			std::vector<RenderGraphNodeHeader*> render_graph{};
		};
	}
}
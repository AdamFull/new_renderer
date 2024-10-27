#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/gfx/Structures.h>
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

		class Renderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddTextureResource(const std::string& path, gfx::Sampler* sampler = nullptr);
			uint32_t AddTextureResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr);

			uint32_t AddBufferResource(const gfx::BufferCreateInfo& create_info);

			uint32_t AddShaderPass(const ComputeShaderPassCreateInfo& create_info);
			uint32_t AddShaderPass(const RenderShaderPassCreateInfo& create_info);

			void Render(float delta_time);

			gfx::Context& GetGFXContext();

			ExecutionContext& GetRenderContext();
			ExecutionContext const& GetRenderContext() const;

			gfx::ResourceContext& GetResourceContext();
			gfx::ResourceContext const& GetResourceContext() const;

			Handle LoadTexture(const std::string& path, gfx::Sampler* sampler = nullptr);
			Handle CreateTexture(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr);
			void DestroyTexture(Handle handle);

			gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info);
		private:
			RenderGraphNodeHeader* AllocateStaticConditionNode();
			RenderGraphNodeHeader* AllocateDynamicConditionNode();
			RenderGraphNodeHeader* AllocateGroupNode();
			RenderGraphNodeHeader* AllocatePassNode();

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
			uint32_t query_index{ 0u };

			utl::hash_map<uint32_t, gfx::Sampler*> sampler_cache{};
			gfx::Sampler* default_sampler{ nullptr };

			//GPUVector<FMaterial> materials{};

			// Render object descriptions
			struct TextureObject
			{
				gfx::Texture* texture{ nullptr };
				gfx::Sampler* sampler{ nullptr };
			};
			ResourceContainer<TextureObject> textures{};
			ResourceContainer<TextureObject> render_targets{};

			struct BufferObject
			{
				gfx::Buffer* buffer;
			};
			ResourceContainer<BufferObject> buffers{};

			// Render graph resources
			std::vector<std::unique_ptr<ShaderPass>> shader_passes{};
			std::vector<RenderGraphPassResourceAccess> per_pass_resouce_access{};

			uint32_t backbuffer_resource{ ~0u };
			std::vector<RenderGraphResource> graph_resources{};

			std::vector<RenderGraphNodeHeader*> render_graph{};


		};
	}
}
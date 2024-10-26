#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/gfx/Structures.h>
#include <engine/rendering/RenderResourceContainer.h>

#include <engine/rendering/GPUVector.h>
#include <engine/rendering/render_graph/ShaderPass.h>
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
		class ExecutionContext;

		class Renderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			void Render(float delta_time);

			gfx::Context& GetGFXContext();

			ExecutionContext& GetRenderContext();
			ExecutionContext const& GetRenderContext() const;

			gfx::ResourceContext& GetResourceContext();
			gfx::ResourceContext const& GetResourceContext() const;

			Handle LoadTexture(const std::string& fath, gfx::Sampler* sampler = nullptr);
			Handle CreateTexture(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr);
			void DestroyTexture(Handle handle);

			gfx::Sampler* GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info);
		private:
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

			// Shader pass description
			std::vector<std::unique_ptr<ShaderPass>> shader_passes{};


		};
	}
}
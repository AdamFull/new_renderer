#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <memory>

namespace engine
{
	class Window;

	namespace gfx
	{
		class Context;
		class QueryPool;
		class Sampler;
	}

	namespace render
	{
		class RenderContext;
		class UploadContext;

		class Renderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			void Render(float delta_time);

			gfx::Context& GetGFXContext();

			RenderContext& GetRenderContext();
			RenderContext const& GetRenderContext() const;

			UploadContext& GetUploadContext();
			UploadContext const& GetUploadContext() const;
		private:
			void BeginFrame();
			void EndFrame();
			void UpdateRenderTargets();
		private:
			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<RenderContext> render_context{};
			std::unique_ptr<UploadContext> upload_context{};

			gfx::QueryPool* timestamp_query{ nullptr };
			uint32_t query_index{ 0u };

			gfx::Sampler* default_sampler{ nullptr };
		};
	}
}
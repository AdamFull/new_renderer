#pragma once

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
		class Renderer
		{
		public:
			Renderer(Window* window);
			~Renderer();
		private:
			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<RenderContext> render_context{};
			std::unique_ptr<UploadContext> upload_context{};

			gfx::QueryPool* timestamp_query{ nullptr };
			uint32_t query_index{ 0u };

			gfx::Sampler* default_sampler{ nullptr };
		};
	}
}
#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/RenderResourceContainer.h>

#include <engine/rendering/ExecutionContext.h>

//#include <engine/rendering/GPUVector.h>
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
		class Renderer : public IRenderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddRenderResource(const std::string& path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;

			uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) override;

			void Render(float delta_time) override;

			gfx::ResourceContext& GetResourceContext() override;
			gfx::ResourceContext const& GetResourceContext() const override;

			gfx::Context& GetContext() override;

			GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;
			GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;

			IRenderGraph& GetRenderGraph() override;
			IRenderGraph const& GetRenderGraph() const override;

			IRenderWorld& GetRenderWorld() override;
			IRenderWorld const& GetRenderWorld() const override;
		private:
			void BuildGraph();
		private:
			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<gfx::ResourceContext> resource_context{};
			std::unique_ptr<IRenderGraph> render_graph{};
			std::unique_ptr<IRenderWorld> render_world{};

			//GPUVector<FMaterial> materials{};

			std::unordered_map<uint32_t, uint32_t> interleaved_geometry_buffers{};
			std::unordered_map<uint32_t, uint32_t> deinterleaved_geometry_buffers{};

			std::vector<std::unique_ptr<GeometryBuffer>> geometry_buffers{};

			uint32_t backbuffer_resource{ ~0u };
		};
	}
}
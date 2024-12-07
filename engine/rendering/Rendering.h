#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include "RendererInterface.h"
#include "RenderResourceContainer.h"
#include "ExecutionContext.h"

#include "render_graph/ShaderPass.h"
#include <memory>

namespace engine
{
	class Window;

	namespace gfx
	{
		class Context;
		class QueryPool;
		class Sampler;
		class ResourceLoader;
	}

	namespace render
	{
		class Renderer : public IRenderer
		{
		public:
			Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddRenderResource(std::string_view path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddRenderResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;

			uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) override;

			void AddMaterial(const shlib::Material& material) override;

			void Render(float delta_time) override;

			gfx::ResourceLoader& GetResourceLoader() override;
			gfx::ResourceLoader const& GetResourceLoader() const override;

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
			std::unique_ptr<gfx::FilesystemWrapper> fs;

			std::unique_ptr<gfx::Context> gfx_context{};
			std::unique_ptr<gfx::ResourceLoader> resource_loader{};
			std::unique_ptr<IRenderGraph> render_graph{};
			std::unique_ptr<IRenderWorld> render_world{};

			//GPUVector<Material> materials{};

			std::unordered_map<uint32_t, uint32_t> interleaved_geometry_buffers{};
			std::unordered_map<uint32_t, uint32_t> deinterleaved_geometry_buffers{};

			std::vector<std::unique_ptr<GeometryBuffer>> geometry_buffers{};

			uint32_t backbuffer_resource{ ~0u };
		};
	}
}
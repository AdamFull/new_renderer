#pragma once

#include <edge/filesystem/VirtualFileSystemInterface.h>
#include "RendererInterface.h"
#include "RenderResourceContainer.h"

#include "render_graph/ShaderPass.h"
#include <memory>

namespace edge
{
	class Window;
}

namespace engine
{
	namespace render
	{
		class Renderer : public IRenderer
		{
		public:
			Renderer(edge::Window* window, edge::filesystem::IVirtualFileSystemInterface* vfs);
			~Renderer();

			uint32_t AddRenderResource(std::string_view path, edge::gfx::Sampler* sampler = nullptr) override;
			uint32_t AddRenderResource(const TextureResourceCreateInfo& create_info, edge::gfx::Sampler* sampler = nullptr) override;

			uint32_t AddRenderResource(const edge::gfx::BufferCreateInfo& create_info) override;

			void AddMaterial(const shlib::Material& material) override;

			void Render(float delta_time) override;

			edge::gfx::ext::ResourceLoader& GetResourceLoader() override;
			edge::gfx::ext::ResourceLoader const& GetResourceLoader() const override;

			edge::gfx::ext::ShaderEffectSystem& GetShaderEffectSystem() override;
			edge::gfx::ext::ShaderEffectSystem const& GetShaderEffectSystem() const override;

			edge::gfx::Context& GetContext() override;

			GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;
			GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) override;

			ExecutionContext& GetRenderContext() override;
			ExecutionContext const& GetRenderContext() const override;

			IRenderWorld& GetRenderWorld() override;
			IRenderWorld const& GetRenderWorld() const override;
		private:
			void BuildGraph();
		private:
			std::unique_ptr<edge::gfx::FilesystemWrapper> fs;

			std::unique_ptr<edge::gfx::Context> gfx_context{};
			std::unique_ptr<edge::gfx::ext::ResourceLoader> resource_loader{};
			std::unique_ptr<edge::gfx::ext::ShaderEffectSystem> shader_effect_system{};
			std::unique_ptr<ExecutionContext> render_context{};
			std::unique_ptr<IRenderWorld> render_world{};

			//GPUVector<Material> materials{};

			std::unordered_map<uint32_t, uint32_t> interleaved_geometry_buffers{};
			std::unordered_map<uint32_t, uint32_t> deinterleaved_geometry_buffers{};

			std::vector<std::unique_ptr<GeometryBuffer>> geometry_buffers{};

			uint32_t backbuffer_resource{ ~0u };
		};
	}
}
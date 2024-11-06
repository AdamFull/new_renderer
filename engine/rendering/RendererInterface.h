#pragma once

#include <engine/rendering/GeometryBufferInterface.h>
#include <engine/rendering/render_graph/RenderGraphInterface.h>
#include <engine/rendering/render_world/RenderWorldInterface.h>

namespace engine
{
	namespace render
	{
		class ExecutionContext;

		class IRenderer
		{
		public:
			virtual ~IRenderer() = default;

			virtual void Render(float delta_time) = 0;

			virtual gfx::ResourceContext& GetResourceContext() = 0;
			virtual gfx::ResourceContext const& GetResourceContext() const = 0;

			virtual uint32_t AddRenderResource(const std::string& path, gfx::Sampler* sampler = nullptr) = 0;
			virtual uint32_t AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler = nullptr) = 0;

			virtual uint32_t AddRenderResource(const gfx::BufferCreateInfo& create_info) = 0;

			virtual void AddMaterial(const FMaterial& material) = 0;

			virtual gfx::Context& GetContext() = 0;

			virtual GeometryBuffer& RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
			virtual GeometryBuffer& RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;

			virtual IRenderGraph& GetRenderGraph() = 0;
			virtual IRenderGraph const& GetRenderGraph() const = 0;

			virtual IRenderWorld& GetRenderWorld() = 0;
			virtual IRenderWorld const& GetRenderWorld() const = 0;
		};
	}
}
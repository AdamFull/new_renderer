#pragma once

#include "ExecutionContext.h"
#include "GeometryBufferInterface.h"
#include "render_world/RenderWorldInterface.h"

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

			virtual uint32_t AddRenderResource(std::string_view path, edge::gfx::ISampler* sampler = nullptr) = 0;
			virtual uint32_t AddRenderResource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler = nullptr) = 0;

			virtual uint32_t AddRenderResource(const edge::gfx::BufferCreateInfo& create_info) = 0;

			virtual void AddMaterial(const shlib::Material& material) = 0;

			virtual edge::gfx::IContext& GetContext() = 0;

			virtual uint32_t RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
			virtual uint32_t RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info) = 0;
			virtual GeometryBuffer& LookUpGeometryBuffer(uint32_t geometry_buffer_id) = 0;


			virtual ExecutionContext& GetRenderContext() = 0;
			virtual ExecutionContext const& GetRenderContext() const = 0;

			virtual IRenderWorld& GetRenderWorld() = 0;
			virtual IRenderWorld const& GetRenderWorld() const = 0;
		};
	}
}
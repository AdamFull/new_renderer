#pragma once

#include "ExecutionContext.h"
#include "GeometryBufferInterface.h"
#include "render_world/RenderWorldInterface.h"

namespace engine::render {
	class IExecutionContext;

	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		virtual void render(float delta_time) = 0;

		virtual auto add_render_resource(std::string_view path, edge::gfx::ISampler* sampler = nullptr) -> uint32_t = 0;
		virtual auto add_render_resource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler = nullptr) -> uint32_t = 0;

		virtual auto add_render_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t = 0;

		virtual void add_material(const shlib::Material& material) = 0;

		virtual auto request_interleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t = 0;
		virtual auto request_deinterleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t = 0;
		virtual auto lookup_geometry_buffer(uint32_t geometry_buffer_id) -> GeometryBuffer & = 0;


		virtual auto get_render_context() -> IExecutionContext & = 0;
		virtual auto get_render_context() const ->IExecutionContext const& = 0;

		virtual auto get_render_world() -> IRenderWorld & = 0;
		virtual auto get_render_world() const ->IRenderWorld const& = 0;
	};
}
#pragma once

#include "RendererInterface.h"
#include "RenderResourceContainer.h"

#include "render_graph/ShaderPass.h"
#include <memory>

namespace edge {
	class Window;
}

namespace engine::render {
	class Renderer : public IRenderer {
	public:
		Renderer(edge::Window* window);
		~Renderer();

		auto add_render_resource(std::string_view path) -> uint32_t override;
		auto add_render_resource(const TextureResourceCreateInfo& create_info) -> uint32_t override;
		auto add_render_resource(const edge::gfx::SamplerCreateInfo& create_info) -> uint32_t override;
		auto add_render_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t override;

		void add_material(const interop::Material& material) override;

		void render(float delta_time) override;

		auto request_interleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t override;
		auto request_deinterleaved_geometry_buffer(const GeometryBufferCreateInfo& create_info) -> uint32_t override;
		auto lookup_geometry_buffer(uint32_t geometry_buffer_id) -> GeometryBuffer& override;

		auto get_render_context() -> IExecutionContext& override;
		auto get_render_context() const ->IExecutionContext const& override;

		auto get_render_world() -> IRenderWorld& override;
		auto get_render_world() const -> IRenderWorld const& override;
	private:
		void build_frame_graph();
		void build_graph();
	private:
		std::unique_ptr<IExecutionContext> render_context{};
		std::unique_ptr<IRenderWorld> render_world{};

		//GPUVector<Material> materials{};

		std::unordered_map<uint32_t, uint32_t> interleaved_geometry_buffers{};
		std::unordered_map<uint32_t, uint32_t> deinterleaved_geometry_buffers{};

		std::vector<std::unique_ptr<GeometryBuffer>> geometry_buffers{};

		uint32_t backbuffer_resource{ ~0u };

		uint32_t input_target_id{ ~0u };
		uint32_t output_target_id{ ~0u };
		uint32_t current_input_target_id{ ~0u };
		uint32_t current_output_target_id{ ~0u };
		uint32_t last_output_target_id{ ~0u };
	};
}
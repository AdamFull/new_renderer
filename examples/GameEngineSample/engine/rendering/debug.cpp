#include "debug.h"

#include <edge/gfx/gfx_context.h>

#include "render_graph/ShaderPass.h"

namespace engine::debug {
	constexpr size_t k_max_words{ 1024ull };

	static DebugDrawContext ctx;

	void create_debug_draw_context() {
		assert(!ctx.text_buffer && "Already initialized.");

		edge::gfx::BufferCreateInfo create_info{};
		create_info.size = sizeof(debug_text::DebugTextData) * k_max_words;
		create_info.usage = edge::gfx::BufferUsageFlag::eStorageBufferCPU;

		ctx.text_buffer = edge::gfx::create_buffer(create_info);
		ctx.text_buffer->set_resource_name("debug_draw_text_buffer");

		// Create vertex buffers
		static glm::vec2 vertex_positions[] = {
				{0, 16}, {0, 0},
				{8 * 4, 0}, {8 * 4, 16}
		};

		static uint16_t indices[] = {
			0, 1, 2,
			0, 2, 3,
		};

		create_info.size = sizeof(glm::vec2) * std::size(vertex_positions);
		create_info.usage = edge::gfx::BufferUsageFlag::eVertexBufferCPU;
		ctx.vertex_buffer = edge::gfx::create_buffer(create_info);
		ctx.vertex_buffer->update(vertex_positions, create_info.size);

		create_info.size = sizeof(uint16_t) * std::size(indices);
		create_info.usage = edge::gfx::BufferUsageFlag::eIndexBufferCPU;
		ctx.index_buffer = edge::gfx::create_buffer(create_info);
		ctx.index_buffer->update(indices, create_info.size);

		// TODO:
		ctx.viewport_resolution = { 1280, 720 };
	}

	void shutdown_debug_draw_context() {
		edge::gfx::destroy(ctx.text_buffer);
		edge::gfx::destroy(ctx.vertex_buffer);
		edge::gfx::destroy(ctx.index_buffer);
	}

	void draw_text(const glm::u16vec2& pos, std::string_view text, float scale, glm::u8vec4 fg_color, glm::u8vec4 bg_color) {
		int32_t text_len = static_cast<int32_t>(text.size());
		int32_t word_count = (text_len + 4) / 4;

		for (int32_t word_idx = 0; word_idx < word_count; ++word_idx) {
			int32_t word_offset = word_idx * 4;
			int32_t char_remain = text_len - word_offset;

			uint32_t word{ 0 };
			memcpy(&word, text.data() + word_offset, char_remain < 4 ? char_remain : 4);

			ctx.text_data.push_back({
					.word = word,
					.pos = {8 * 4 * word_idx * scale + pos.x, pos.y},
					.scale = scale,
					.fg_color = fg_color,
					.bg_color = bg_color
				});
		}
	}

	void flush_draw_data() {
		draw_text({ 100, 100 }, "Hello World!", 2.0f);

		if (!ctx.text_data.empty()) {
			ctx.text_buffer->update(ctx.text_data.data(), ctx.text_data.size() * sizeof(debug_text::DebugTextData));
		}
	}

	void draw_cb(render::ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time) {
		flush_draw_data();

		command_list.set_scissor(0, 0, static_cast<uint32_t>(ctx.viewport_resolution.x), static_cast<uint32_t>(ctx.viewport_resolution.y));
		command_list.set_viewport(0.f, 0.f, ctx.viewport_resolution.x, ctx.viewport_resolution.y, 0.f, 1.f);

		// Bind vertex and index buffers
		uint32_t vert_stride{ 0 };
		uint64_t vert_offset{ 0 };
		command_list.bind_vertex_buffers({ &ctx.vertex_buffer, 1 }, { &vert_stride, 1 }, { &vert_offset, 1 });
		command_list.bind_index_buffer(ctx.index_buffer, edge::gfx::IndexType::eUint16, 0);

		auto& pc = self.get_constant_block();
		auto& pc_data = pc.as<debug_text::PushConstants>();
		pc_data.word_data = ctx.text_buffer->get_address();
		pc_data.inv_viewport_resolution = 1.f / ctx.viewport_resolution;
		self.push_constant_ranges(command_list);
		self.resolve_dependencies(command_list);

		self.begin(command_list);
		command_list.draw_indexed(6u, 0u, static_cast<uint32_t>(ctx.text_data.size()), 0u, 0u);
		self.end(command_list);

		ctx.text_data.clear();
	}

	DebugDrawContext* get_debug_draw_context() {
		return &ctx;
	}
}
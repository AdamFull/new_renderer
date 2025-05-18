#pragma once

#include <string_view>
#include <glm/glm.hpp>

#include <edge/gfx/gfx_base.h>

#include "../../assets/shaders/testing/debug_text.h"

namespace engine::render {
	class ShaderPass;
}

namespace engine::debug {
	namespace color {
		constexpr glm::u8vec4 k_white{ 255, 255, 255, 255 };
		constexpr glm::u8vec4 k_black{ 0, 0, 0, 0 };
	}

	struct DebugDrawContext {
		edge::gfx::IBuffer* text_buffer{ nullptr };

		edge::gfx::IBuffer* vertex_buffer{ nullptr };
		edge::gfx::IBuffer* index_buffer{ nullptr };

		glm::vec2 viewport_resolution{};

		std::vector<debug_text::DebugTextData> text_data{};
	};

	void create_debug_draw_context();
	void shutdown_debug_draw_context();

	void draw_text(const glm::u16vec2& pos, std::string_view text, float scale = 1.5f, glm::u8vec4 fg_color = color::k_white, glm::u8vec4 bg_color = color::k_black);
	void flush_draw_data();

	void draw_cb(render::ShaderPass& self, edge::gfx::ICommandList& command_list, float delta_time);

	DebugDrawContext* get_debug_draw_context();
}
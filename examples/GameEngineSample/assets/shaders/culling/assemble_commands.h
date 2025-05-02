#pragma once
#include "../include/interop.h"

namespace assemble_commands {
	struct PushConstants {
		TYPE_PTR(interop::DrawDescription) source_descriptions;
		TYPE_PTR(interop::DrawDescription) alive_descriptions;
		TYPE_PTR(interop::DrawIndexedIndirectData) alive_draw_commands;
		TYPE_PTR(uint32_t) alive_bits;
		TYPE_PTR(uint32_t) alive_offsets;
		TYPE_PTR(uint32_t) draw_counts;
	};
}
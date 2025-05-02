#pragma once

#include "../include/interop.h"

namespace copy_buffer {
	struct PushConstant {
		TYPE_PTR(uint32_t) copy_src;
		TYPE_PTR(uint32_t) copy_dst;
	};
}
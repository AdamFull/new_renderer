#pragma once

#include "../include/interop.h"

namespace cull_and_batch {
    struct PushConstants {
        TYPE_PTR(interop::DrawDescription) input_draw_desc_ptr;
        TYPE_PTR(interop::DrawIndexedIndirectData) output_draw_indirect_ptr;
        TYPE_PTR(uint32_t) visibility_ptr;
        uint32_t flags;
        uint32_t _pad0;
    };
}
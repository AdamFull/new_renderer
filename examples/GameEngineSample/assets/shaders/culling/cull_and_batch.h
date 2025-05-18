#pragma once

#include "../include/interop.h"

namespace cull_and_batch {
    struct PushConstants {
        TYPE_PTR(interop::DrawDescription) input_draw_desc_ptr;
        TYPE_PTR(interop::DrawIndexedIndirectData) output_draw_indirect_ptr;
        TYPE_PTR(uint32_t) visibility_ptr;
        uint64_t _pad0;
        int32_t prepass;
        int32_t frustum_culling_enabled;
        int32_t occlusion_culling_enabled;
        int32_t _pad1;
    };
}
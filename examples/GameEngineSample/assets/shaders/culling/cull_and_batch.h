#pragma once

#include "../include/interop.h"

namespace cull_and_batch {
    struct PushConstants {
        TYPE_PTR(interop::DrawDescription) draw_desc_ptr;
        TYPE_PTR(uint32_t) alive_meshes_bit_array_ptr;
    };
}
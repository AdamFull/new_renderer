#pragma once

#include "../include/interop.h"

namespace scan {
    struct PushConstants {
        TYPE_PTR(uint32_t) input_buffer;
        TYPE_PTR(uint32_t) output_buffer;
    };
}
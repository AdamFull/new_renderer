#pragma once
#include "../include/interop.h"

namespace debug_text {
    struct DebugTextData {
        uint32_t word;
        int16_t2 pos;
        float scale;
        uint8_t4 fg_color;
        uint8_t4 bg_color;
    };

    struct PushConstants {
        DebugTextData* word_data;
        float2 inv_viewport_resolution;
    };
}
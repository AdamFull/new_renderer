#pragma once

#include "../include/interop.h"

namespace clear_buffer {
    struct PushConstants {
        TYPE_PTR(uint32_t) word_ptr;
        uint64_t size_in_words;
    };
}
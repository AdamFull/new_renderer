#include "clear_buffer.h"

#define GROUP_SIZE 256

[[vk::push_constant]] ConstantBuffer<clear_buffer::PushConstants> pc;

[shader("compute")]
[numthreads(GROUP_SIZE, 1, 1)]
void main(uint32_t3 global_id: SV_DispatchThreadID) {
    if (global_id.x < pc.size_in_words) {
        pc.word_ptr[global_id.x] = 0u;
    }
}
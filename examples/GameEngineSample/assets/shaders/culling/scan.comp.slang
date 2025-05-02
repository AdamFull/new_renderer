#include "scan.h"

[[vk::push_constant]] ConstantBuffer<scan::PushConstants> pc;

#define GROUP_SIZE 256

groupshared uint32_t temp[GROUP_SIZE * 2];

uint32_t sample_bit(uint32_t global_id)
{
    // Handle bit array input
    uint32_t word_index = global_id >> 5; // Divide by 32
    uint32_t bit_index = global_id & 31;  // Modulo 32
    uint32_t word = pc.input_buffer[word_index];
    return (word >> bit_index) & 1u;
}

[shader("compute")]
[numthreads(GROUP_SIZE, 1, 1)]
void main(uint32_t3 global_id : SV_DispatchThreadID, uint32_t3 local_id: SV_GroupThreadID) {
    uint32_t n = GROUP_SIZE * 2;

    // Compute global indices.
    // If each workgroup handles n elements, the global index for the data:
    uint32_t global_index = global_id.x * 2;

    // Load input into shared memory:
    // Note: the original CUDA code shows temp[2*local_id.x+1] commented out, but presumably it should be loaded as well.
#ifdef INPUT_BIT_ARRAY
    temp[2 * local_id.x] = sample_bit(global_index);
    temp[2 * local_id.x + 1] = sample_bit(global_index + 1);
#else
    temp[2 * local_id.x] = pc.input_buffer[global_index];
    temp[2 * local_id.x + 1] = pc.input_buffer[global_index + 1];
#endif

    int offset = 1;

    // Build sum in place up the tree
    for (int d = int(n >> 1); d > 0; d >>= 1) {
        GroupMemoryBarrierWithGroupSync(); // sync threads
        if (int(local_id.x) < d) {
            int ai = offset * (2 * int(local_id.x) + 1) - 1;
            int bi = offset * (2 * int(local_id.x) + 2) - 1;
            temp[bi] += temp[ai];
        }
        offset *= 2;
    }

    // Clear the last element
    if (local_id.x == 0) {
        temp[n - 1] = 0u;
    }

    // Traverse down tree and build scan
    for (int d = 1; d < int(n); d *= 2) {
        offset >>= 1;
        GroupMemoryBarrierWithGroupSync(); // sync threads
        if (int(local_id.x) < d)
        {
            int ai = offset * (2 * int(local_id.x) + 1) - 1;
            int bi = offset * (2 * int(local_id.x) + 2) - 1;
            uint32_t t = temp[ai];
            temp[ai] = temp[bi];
            temp[bi] += t;
        }
    }

    GroupMemoryBarrierWithGroupSync(); // ensure all data is ready

    // Write results to global memory
    pc.output_buffer[global_index] = temp[2 * local_id.x];
    pc.output_buffer[global_index + 1] = temp[2 * local_id.x + 1];
}
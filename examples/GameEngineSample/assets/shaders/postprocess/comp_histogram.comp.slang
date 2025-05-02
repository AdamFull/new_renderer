// Based on shader from this article https://bruop.github.io/exposure/

#include "comp_histogram.h"

#define THREADS_X 16
#define THREADS_Y 16
#define GROUP_SIZE THREADS_X * THREADS_Y

#define EPSILON 0.005
#define RGB_TO_LUM float3(0.2125, 0.7154, 0.0721)

Texture2D texture0;
SamplerState sampler0;

[[vk::push_constant]] ConstantBuffer<comp_histogram::PushConstant> pc;

// Shared histogram buffer used for storing intermediate sums for each work group
groupshared uint32_t histogram_shared[GROUP_SIZE];

// For a given color and luminance range, return the histogram bin index
uint color_to_bin(float3 hdrColor, float fMinLogLum, float inverseLogLumRange) {
    // Convert our RGB value to Luminance, see note for RGB_TO_LUM macro above
    float lum = dot(hdrColor, RGB_TO_LUM);

    // Avoid taking the log of zero
    if (lum < EPSILON) {
        return 0;
    }

    // Calculate the log_2 luminance and express it as a value in [0.0, 1.0]
    // where 0.0 represents the minimum luminance, and 1.0 represents the max.
    float logLum = clamp((log2(lum) - fMinLogLum) * inverseLogLumRange, 0.0, 1.0);

    // Map [0, 1] to [1, 255]. The zeroth bin is handled by the epsilon check above.
    return uint32_t(logLum * 254.0 + 1.0);
}

[numthreads(THREADS_X, THREADS_Y, 1)]
[shader("compute")]
void main(uint3 global_id: SV_DispatchThreadID, uint32_t local_index: SV_GroupIndex) {
    // Initialize the bin for this thread to 0
    histogram_shared[local_index] = 0;
    GroupMemoryBarrierWithGroupSync();

    uint32_t2 dim;
    texture0.GetDimensions(dim.x, dim.y);

    // Ignore threads that map to areas beyond the bounds of our HDR image
    if (global_id.x < dim.x && global_id.y < dim.y) {
        float3 hdr_color = texture0.Load(int32_t3(global_id.xy, 0)).xyz;
        uint32_t bin_index = color_to_bin(hdr_color, pc.min_log_lum, 1.0 / pc.log_lum_range);

        // We use an atomic add to ensure we don't write to the same bin in our
        // histogram from two different threads at the same time.
        InterlockedAdd(histogram_shared[bin_index], 1);
    }

    // Wait for all threads in the work group to reach this point before adding our
    // local histogram to the global one
    GroupMemoryBarrierWithGroupSync();

    // Technically there's no chance that two threads write to the same bin here,
    // but different work groups might! So we still need the atomic add.
    InterlockedAdd(pc.histogram[local_index], histogram_shared[local_index]);
}
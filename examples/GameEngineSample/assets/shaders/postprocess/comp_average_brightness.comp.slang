// Based on shader from this article https://bruop.github.io/exposure/

#include "comp_histogram.h"

#define THREADS_X 256
#define THREADS_Y 1
#define GROUP_SIZE THREADS_X *THREADS_Y

RWTexture1D<float> image0;

[[vk::push_constant]] ConstantBuffer<comp_brightness::PushConstant> pc;

groupshared uint histogram_shared[GROUP_SIZE];

[numthreads(THREADS_X, THREADS_Y, 1)]
[shader("compute")]
void main(uint3 global_id: SV_DispatchThreadID, uint32_t local_index : SV_GroupIndex) {

    // Get the count from the histogram buffer
    uint32_t count_for_this_bin = pc.histogram[local_index];
    histogram_shared[local_index] = count_for_this_bin * local_index;

    GroupMemoryBarrierWithGroupSync();

    // Reset the count stored in the buffer in anticipation of the next pass
    pc.histogram[local_index] = 0;

    // This loop will perform a weighted count of the luminance range
    for (uint cutoff = (GROUP_SIZE >> 1); cutoff > 0; cutoff >>= 1) {
        if (local_index < cutoff) {
            histogram_shared[local_index] += histogram_shared[local_index + cutoff];
        }

        GroupMemoryBarrierWithGroupSync();
    }

    // We only need to calculate this once, so only a single thread is needed.
    if (global_id.x == 0) {
        // Here we take our weighted sum and divide it by the number of pixels
        // that had luminance greater than zero (since the index == 0, we can
        // use count_for_this_bin to find the number of black pixels)
        float weightedLogAverage = (histogram_shared[0] / max((pc.num_pixels_x * pc.num_pixels_y) - float(count_for_this_bin), 1.0)) - 1.0;

        // Map from our histogram space to actual luminance
        float weighted_avg_lum = exp2(((weightedLogAverage / 254.0) * pc.log_lum_range) + pc.min_log_lum);

        // The new stored value will be interpolated using the last frames value
        // to prevent sudden shifts in the exposure.
        float lum_last_frame = image0.Load(0);
        float adapted_lum = lum_last_frame + (weighted_avg_lum - lum_last_frame) * pc.time_coeff;
        image0.Store(0, adapted_lum);
    }
}
#pragma once

// Source https://developer.nvidia.com/blog/optimizing-compute-shaders-for-l2-locality-using-thread-group-id-swizzling/

/*
 * Copyright (c) 2020-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// Divide the 2D-Dispatch_Grid into tiles of dimension [N, DipatchGridDim.y]
// “CTA” (Cooperative Thread Array) == Thread Group in DirectX terminology
inline uint32_t2 thread_group_tiling_x(
    const uint32_t2 dipatch_grid_dim, // Arguments of the Dispatch call (typically from a ConstantBuffer)
    const uint32_t2 cta_dim,          // Already known in HLSL, eg:[numthreads(8, 8, 1)] -> uint2(8, 8)
    const uint32_t max_tile_width,    // User parameter (N). Recommended values: 8, 16 or 32.
    const uint32_t2 group_thread_id,  // SV_GroupThreadID
    const uint32_t2 group_id          // SV_GroupID
) {
    // A perfect tile is one with dimensions = [max_tile_width, dipatch_grid_dim.y]
    const uint32_t number_of_ctas_in_a_perfect_tile = max_tile_width * dipatch_grid_dim.y;

    // Possible number of perfect tiles
    const uint32_t number_of_perfect_tiles = dipatch_grid_dim.x / max_tile_width;

    // Total number of CTAs present in the perfect tiles
    const uint32_t total_ctas_in_all_perfect_tiles = number_of_perfect_tiles * max_tile_width * dipatch_grid_dim.y;
    const uint32_t v_thread_group_id_flattened = dipatch_grid_dim.x * group_id.y + group_id.x;

    // tile_id_of_current_cta : current CTA to TILE-ID mapping.
    const uint32_t tile_id_of_current_cta = v_thread_group_id_flattened / number_of_ctas_in_a_perfect_tile;
    const uint32_t local_cta_id_within_current_tile = v_thread_group_id_flattened % number_of_ctas_in_a_perfect_tile;
    uint32_t local_cta_id_y_within_current_tile;
    uint32_t local_cta_id_x_within_current_tile;

    if (total_ctas_in_all_perfect_tiles <= v_thread_group_id_flattened) {
        // Path taken only if the last tile has imperfect dimensions and CTAs from the last tile are launched.
        uint32_t x_dimension_of_last_tile = dipatch_grid_dim.x % max_tile_width;
#ifdef DXC_STATIC_DISPATCH_GRID_DIM
        x_dimension_of_last_tile = max(1, x_dimension_of_last_tile);
#endif
        local_cta_id_y_within_current_tile = local_cta_id_within_current_tile / x_dimension_of_last_tile;
        local_cta_id_x_within_current_tile = local_cta_id_within_current_tile % x_dimension_of_last_tile;
    }
    else {
        local_cta_id_y_within_current_tile = local_cta_id_within_current_tile / max_tile_width;
        local_cta_id_x_within_current_tile = local_cta_id_within_current_tile % max_tile_width;
    }

    const uint32_t swizzled_v_thread_group_id_flattened =
		tile_id_of_current_cta * max_tile_width +
                                                          local_cta_id_y_within_current_tile * dipatch_grid_dim.x +
                                                          local_cta_id_x_within_current_tile;

    uint32_t2 swizzledv_thread_group_id;
    swizzledv_thread_group_id.y = swizzled_v_thread_group_id_flattened / dipatch_grid_dim.x;
    swizzledv_thread_group_id.x = swizzled_v_thread_group_id_flattened % dipatch_grid_dim.x;

    uint32_t2 swizzledv_thread_id;
    swizzledv_thread_id.x = cta_dim.x * swizzledv_thread_group_id.x + group_thread_id.x;
    swizzledv_thread_id.y = cta_dim.y * swizzledv_thread_group_id.y + group_thread_id.y;

    return swizzledv_thread_id.xy;
}
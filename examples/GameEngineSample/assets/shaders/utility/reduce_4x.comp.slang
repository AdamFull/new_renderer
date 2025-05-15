#include "reduce_4x.h"

#define TG_SIZE_X 8
#define TG_SIZE_Y 8
#define TG_CACHE_SIZE (TG_SIZE_X * TG_SIZE_Y)

#ifdef DEPTH_REDUCE
typedef float color_t;
#define COMPONENTS r
#else
typedef float4 color_t;
#define COMPONENTS rgba
#endif

WTexture2D<color_t> image0[16];

Texture2D<color_t> texture0;
SamplerState sampler0;

[[vk::push_constant]] ConstantBuffer<reduce::PushConstants> pc;

#ifdef DEPTH_REDUCE
groupshared float color_r[TG_CACHE_SIZE];
#else
groupshared float color_r[TG_CACHE_SIZE];
groupshared float color_g[TG_CACHE_SIZE];
groupshared float color_b[TG_CACHE_SIZE];
groupshared float color_a[TG_CACHE_SIZE];
#endif

void store_color(uint32_t id, color_t in_color) {
#ifdef DEPTH_REDUCE
    color_r[id] = in_color;
#else
    color_r[id] = in_color.r;
    color_g[id] = in_color.g;
    color_b[id] = in_color.b;
    color_a[id] = in_color.a;
#endif
}

color_t load_color(uint32_t id) {
#ifdef DEPTH_REDUCE
    return color_r[id];
#else
    return color_t(color_r[id], color_g[id], color_b[id], color_a[id]);
#endif
}

inline color_t reduction_finction(color_t c0, color_t c1, color_t c2, color_t c3) {
#ifdef DEPTH_REDUCE
    return min(min(c0, c1), min(c2, c3));
#else
    return 0.25 * (c0 + c1 + c2 + c3);
#endif
}

[numthreads(TG_SIZE_X, TG_SIZE_Y, 1)]
[shader("compute")]
void main(uint3 global_id: SV_DispatchThreadID, uint32_t group_id: SV_GroupIndex) {
    float2 uv = (float2(global_id.xy) + 0.5) * pc.texel_size;

    // TODO: Need to use specific sampler settings and pass current sample level
    color_t color0 = texture0.SampleLevel(sampler0, uv, pc.mip_level).COMPONENTS;
    image0[pc.mip_level + 1].Store(global_id.xy, color0);

    if (pc.mip_count == 1) {
        return;
    }

    store_color(group_id, color0);

    // Barrier for group-shared writes
    GroupMemoryBarrierWithGroupSync();

    // If X and Y are even => (group_id & 0x9) == 0
    if ((group_id & 0x9) == 0) {
        color_t color1 = load_color(group_id + 0x01);
        color_t color2 = load_color(group_id + 0x08);
        color_t color3 = load_color(group_id + 0x09);

        color0 = reduction_finction(color0, color1, color2, color3);

        // Write the 2x2 reduced color
        image0[pc.mip_level + 2].Store(global_id.xy >> 1, color0);

        // Store again in shared memory, as the next pass depends on it
        store_color(group_id, color0);
    }

    if (pc.mip_count == 2) {
        return;
    }

    GroupMemoryBarrierWithGroupSync();

    // If X and Y are multiples of four => (group_id & 0x1B) == 0
    //   0x1B in binary is 11011, but the relevant bits are in the low 6 bits
    //   This check means GTid.x is multiple of 4 and GTid.y is multiple of 4.
    if ((group_id & 0x1B) == 0) {
        color_t color1 = load_color(group_id + 0x02);
        color_t color2 = load_color(group_id + 0x10);
        color_t color3 = load_color(group_id + 0x12);

        color0 = reduction_finction(color0, color1, color2, color3);

        // Write the 4x4 reduced color
        image0[pc.mip_level + 3].Store(global_id.xy >> 2, color0);

        store_color(group_id, color0);
    }

    if (pc.mip_count == 3) {
        return;
    }

    GroupMemoryBarrierWithGroupSync();

    // If X and Y are multiples of eight => only GIid == 0 in an 8x8 group
    if (group_id == 0) {
        color_t color1 = load_color(group_id + 0x04);
        color_t color2 = load_color(group_id + 0x20);
        color_t color3 = load_color(group_id + 0x24);

        color0 = reduction_finction(color0, color1, color2, color3);

        // Write the 8x8 reduced color
        image0[pc.mip_level + 4].Store(global_id.xy >> 3, color0);
    }
}
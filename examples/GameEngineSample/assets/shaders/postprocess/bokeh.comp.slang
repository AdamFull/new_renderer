#include "bokeh.h"
#include "../include/compute_common.slang"
#include "../include/constants.h"

Texture2D texture0;
SamplerState sampler0;
WTexture2D<float4> image0;

[[vk::push_constant]] ConstantBuffer<bokeh::PushConstant> pc;

inline float4 gamma_fun(float4 vector, float gamma) {
    return float4(pow(vector.rgb, float3(gamma)), vector.w);
}

[numthreads(WG_TILE_SIZE, WG_TILE_SIZE, 1)]
[shader("compute")]
void main(uint32_t3 local_id: SV_GroupThreadID,
          uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 group_id: SV_GroupID) {
    if (pc.enabled < 0) {
        return;
    }

    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    float2 texel_size = 1.0f / float2(image_size);

    uint32_t2 px;
    float2 uv;

    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    float4 color = float4(0.0);
    for (float x = -pc.sample_count; x <= pc.sample_count; x += 1.0) {
        for (float y = -pc.sample_count; y <= pc.sample_count; y += 1.0) {
            float2 offset = float2(x, y);
            float angle = atan2(y, x);
            float radius = sqrt(dot(offset, offset));

            float para_poly = cos(angle - (constants::two_pi * floor((constants::pi + pc.polygon_count * angle) / constants::two_pi)) / pc.polygon_count) / cos(constants::pi / pc.polygon_count);
            float sdf_poly = para_poly * radius - pc.sample_count;
            float egdeFalloff = smoothstep(0.0, -1.0, sdf_poly);

            float4 tap = texture0.SampleLevel(sampler0, uv + texel_size * offset.xy, 0.0f);
            tap = gamma_fun(tap, 4.0);
            color += tap * egdeFalloff;
        }
    }

    color.rgb /= color.www;
    color = gamma_fun(color, 1.0 / 4.0);

    image0.Store(px, float4(color.rgb, 1.0));
}
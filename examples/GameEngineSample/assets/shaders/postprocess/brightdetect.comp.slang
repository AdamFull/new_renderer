#include "brightdetect.h"
#include "../include/compute_common.slang"

Texture2D texture0;
SamplerState sampler0;
WTexture2D<float4> image0;

[[vk::push_constant]] ConstantBuffer<brightdetect::PushConstant> pc;

[numthreads(8, 8, 1)]
[shader("compute")]
void main(uint32_t3 local_id: SV_GroupThreadID,
          uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 group_id: SV_GroupID) {

    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    float2 texel_size = 1.0f / float2(image_size);

    uint32_t2 px;
    float2 uv;

    if (swizzle_workgroups_8x8(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    float3 color = texture0.SampleLevel(sampler0, uv, 0.0f).rgb;

    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    image0.Store(px, select(brightness > pc.threshold, float4(color, 1.0), float4(0.0, 0.0, 0.0, 1.0)));
}
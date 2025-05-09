#include "../include/compute_common.slang"
#include "../include/bindles_layout.slang"

[[vk::binding(0, 1)]] WTexture2D<float4> image0 : register(u0, space1);
[[vk::binding(1, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(2, 1)]] SamplerState sampler0 : register(s0, space1);
[[vk::binding(3, 1)]] RaytracingAccelerationStructure tlas : register(t1, space1);

[numthreads(8, 8, 1)]
[shader("compute")]
void main(uint32_t3 local_id: SV_GroupThreadID,
          uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 group_id: SV_GroupID) {

    uint32_t2 px;
    float2 uv;

    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    float2 texel_size = 1.0f / float2(image_size);

    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }
}
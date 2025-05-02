#include "../include/compute_common.slang"

WTexture2D<float4> image0;
Texture2D texture0;
SamplerState sampler0;

[numthreads(8, 8, 1)]
[shader("compute")]
void main(uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 local_id: SV_GroupThreadID,
          uint32_t3 group_id: SV_GroupID) {

    uint2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);

    uint2 px = uint2(0, 0);
    float2 uv = float2(0.0, 0.0);
    if (swizzle_workgroups_8x8(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    float4 color = texture0.SampleLevel(sampler0, uv, 0.0f);
    image0.Store(px, color);
}
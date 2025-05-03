#include "../include/compute_common.slang"
#include "downsampler.h"

// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!

WTexture2D<float4> image0;
Texture2D texture0;
SamplerState sampler0;

[[vk::push_constant]] ConstantBuffer<downsampler::PushConstant> pc;

[numthreads(WG_TILE_SIZE, WG_TILE_SIZE, 1)]
[shader("compute")]
void main(uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 local_id: SV_GroupThreadID,
          uint32_t3 group_id: SV_GroupID) {

    uint2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);

    uint2 px = uint2(0, 0);
    float2 uv = float2(0.0, 0.0);
    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = pc.filter_radius;
    float y = pc.filter_radius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = texture0.SampleLevel(sampler0, float2(uv.x - x, uv.y + y), pc.current_mip).rgb;
    float3 b = texture0.SampleLevel(sampler0, float2(uv.x, uv.y + y), pc.current_mip).rgb;
    float3 c = texture0.SampleLevel(sampler0, float2(uv.x + x, uv.y + y), pc.current_mip).rgb;

    float3 d = texture0.SampleLevel(sampler0, float2(uv.x - x, uv.y), pc.current_mip).rgb;
    float3 e = texture0.SampleLevel(sampler0, float2(uv.x, uv.y), pc.current_mip).rgb;
    float3 f = texture0.SampleLevel(sampler0, float2(uv.x + x, uv.y), pc.current_mip).rgb;

    float3 g = texture0.SampleLevel(sampler0, float2(uv.x - x, uv.y - y), pc.current_mip).rgb;
    float3 h = texture0.SampleLevel(sampler0, float2(uv.x, uv.y - y), pc.current_mip).rgb;
    float3 i = texture0.SampleLevel(sampler0, float2(uv.x + x, uv.y - y), pc.current_mip).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 color = e * 4.0;
    color += (b + d + f + h) * 2.0;
    color += (a + c + g + i);
    color *= 1.0 / 16.0;

    image0.Store(px, float4(color, 1.0));
}
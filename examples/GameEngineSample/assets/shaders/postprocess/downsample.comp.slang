#include "../include/compute_common.slang"
#include "downsampler.h"

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!

WTexture2D<float4> image0;
Texture2D texture0;
SamplerState sampler0;

[[vk::push_constant]] ConstantBuffer<downsampler::PushConstant> pc;

float3 PowVec3(float3 v, float p) {
    return float3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

static const float inv_gamma = 1.0 / 2.2;
float3 ToSRGB(float3 v) { return PowVec3(v, inv_gamma); }

float sRGBToLuma(float3 col) {
    // return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
    return dot(col, float3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(float3 col) {
    // Formula is 1 / (1 + luma)
    float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

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

        float2 texel_size = 1.0 / float2(image_size);

        float x = texel_size.x;
        float y = texel_size.y;

        // Take 13 samples around current texel:
        // a - b - c
        // - j - k -
        // d - e - f
        // - l - m -
        // g - h - i
        // === ('e' is the current texel) ===
        float3 a = texture0.SampleLevel(sampler0, float2(uv.x - 2 * x, uv.y + 2 * y), pc.current_mip).rgb;
        float3 b = texture0.SampleLevel(sampler0, float2(uv.x, uv.y + 2 * y), pc.current_mip).rgb;
        float3 c = texture0.SampleLevel(sampler0, float2(uv.x + 2 * x, uv.y + 2 * y), pc.current_mip).rgb;

        float3 d = texture0.SampleLevel(sampler0, float2(uv.x - 2 * x, uv.y), pc.current_mip).rgb;
        float3 e = texture0.SampleLevel(sampler0, float2(uv.x, uv.y), pc.current_mip).rgb;
        float3 f = texture0.SampleLevel(sampler0, float2(uv.x + 2 * x, uv.y), pc.current_mip).rgb;

        float3 g = texture0.SampleLevel(sampler0, float2(uv.x - 2 * x, uv.y - 2 * y), pc.current_mip).rgb;
        float3 h = texture0.SampleLevel(sampler0, float2(uv.x, uv.y - 2 * y), pc.current_mip).rgb;
        float3 i = texture0.SampleLevel(sampler0, float2(uv.x + 2 * x, uv.y - 2 * y), pc.current_mip).rgb;

        float3 j = texture0.SampleLevel(sampler0, float2(uv.x - x, uv.y + y), pc.current_mip).rgb;
        float3 k = texture0.SampleLevel(sampler0, float2(uv.x + x, uv.y + y), pc.current_mip).rgb;
        float3 l = texture0.SampleLevel(sampler0, float2(uv.x - x, uv.y - y), pc.current_mip).rgb;
        float3 m = texture0.SampleLevel(sampler0, float2(uv.x + x, uv.y - y), pc.current_mip).rgb;

        // Apply weighted distribution:
        // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
        // a,b,d,e * 0.125
        // b,c,e,f * 0.125
        // d,e,g,h * 0.125
        // e,f,h,i * 0.125
        // j,k,l,m * 0.5
        // This shows 5 square areas that are being sampled. But some of them overlap,
        // so to have an energy preserving downsample we need to make some adjustments.
        // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
        // contribute 0.5 to the final color output. The code below is written
        // to effectively yield this sum. We get:
        // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
        float3 color = float3(0.0);
        float3 groups[5];

#ifdef FIRST_PASS
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = (a + b + d + e) * (0.125f / 4.0f);
        groups[1] = (b + c + e + f) * (0.125f / 4.0f);
        groups[2] = (d + e + g + h) * (0.125f / 4.0f);
        groups[3] = (e + f + h + i) * (0.125f / 4.0f);
        groups[4] = (j + k + l + m) * (0.5f / 4.0f);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        color = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
        color = max(color, 0.0001f);
#else
        color = e * 0.125;
        color += (a + c + g + i) * 0.03125;
        color += (b + d + f + h) * 0.0625;
        color += (j + k + l + m) * 0.125;
#endif

        image0.Store(px, float4(color, 1.0));
    }
#include "../include/compute_common.slang"
#include "../include/lighting.slang"
#include "../include/random.slang"
#include "prefilteredmap.h"

WTexture2DArray<float4> image0;
TextureCube<float4> texture0;
SamplerState sampler0;

[[vk::push_constant]] ConstantBuffer<prefilteredmap::PushConstants> pc;

#define NUM_SAMPLES 32

#define NUM_THREADS_Z 6
[numthreads(WG_TILE_SIZE , WG_TILE_SIZE, NUM_THREADS_Z)]
void main(uint3 global_id: SV_DispatchThreadID, uint3 local_id: SV_GroupThreadID, uint3 group_id: SV_GroupID) {
    uint3 image_size;
    image0.GetDimensions(image_size.x, image_size.y, image_size.z);

    uint3 px = global_id;

    uint2 px_swizzled = uint2(0, 0);
    float2 uv = float2(0, 0);
    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size.xy, uv, px_swizzled))
        return;

    px.xy = px_swizzled;

    float3 R = normalize(get_cube_side_direction(uv, px.z));
    float3 N = R;
    float3 V = R;
    float3 colour = float3(0.0f);
    float totalWeight = 0.0f;

    uint envMapDim;
    texture0.GetDimensions(0, envMapDim, envMapDim, envMapDim);
    float fEnvMapDim = float(envMapDim);

    [unroll]
    for (uint i = 0u; i < NUM_SAMPLES; i++) {
        float2 Xi = random::hammersley2d(i, NUM_SAMPLES);
        float3 H = random::importance_sample_ggx(Xi, pc.roughness, N);
        float3 L = 2.0f * dot(V, H) * H - V;
        float dotNL = saturate(dot(N, L));

        if (dotNL > 0.0f) {
            float dotNH = saturate(dot(N, H));
            float dotVH = saturate(dot(V, H));

            // Probability Distribution Function
            float pdf = d_ggx(dotNH, pc.roughness) * dotNH / (4.0f * dotVH) + 0.0001f;
            // Solid angle of current sample
            float omegaS = 1.0f / (float(NUM_SAMPLES) * pdf);
            // Solid angle of 1 pixel across all cube faces
            float omegaP = constants::four_pi / (6.0f * fEnvMapDim * fEnvMapDim);
            // Biased (+1.0) mip level for better result
            float mipLevel = pc.roughness == 0.0f ? 0.0f : max(0.5f * log2(omegaS / omegaP) + 1.0f, 0.0f);

            colour += texture0.SampleLevel(sampler0, L, mipLevel).rgb * dotNL;
            totalWeight += dotNL;
        }
    }

    image0.Store(px, float4(colour / totalWeight, 1.0f));
}
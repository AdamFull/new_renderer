#include "../include/lighting.slang"
#include "../include/random.slang"
#include "../include/compute_common.slang"

RWTexture2D<float2> image0;

#define NUM_SAMPLES 1024u

#define NUM_THREADS_X 8
#define NUM_THREADS_Y 8
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
void main(uint3 global_id: SV_DispatchThreadID, uint3 local_id: SV_GroupThreadID, uint3 group_id: SV_GroupID) {
    uint2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);

    uint2 px = uint2(0, 0);
    float2 uv = float2(0, 0);
    if (swizzle_workgroups_8x8(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px))
        return;

    // Normal always points along z-axis for the 2D lookup
    const float3 N = float3(0.0f, 0.0f, 1.0f);
    float roughness = 1.0f - uv.y;
    float3 V = float3(sqrt(1.0f - uv.x * uv.x), 0.0f, uv.x);

    float2 LUT = float2(0.0f);

    [unroll]
    for (uint i = 0u; i < NUM_SAMPLES; i++) {
        float2 Xi = random::hammersley2d(i, NUM_SAMPLES);
        float3 H = random::importance_sample_ggx(Xi, roughness, N);
        float3 L = 2.0f * dot(V, H) * H - V;

        float dotNL = max(dot(N, L), 0.0f);
        float dotNV = max(dot(N, V), 0.0f);
        float dotVH = max(dot(V, H), 0.0f);
        float dotNH = max(dot(H, N), 0.0f);

        if (dotNL > 0.0f)
        {
            float G = g_schlicksmith_ggx(dotNL, dotNV, roughness);
            float G_Vis = (G * dotVH) / (dotNH * dotNV);
            float Fc = pow(1.0f - dotVH, 5.0f);
            LUT += float2((1.0f - Fc) * G_Vis, Fc * G_Vis);
        }
    }

    image0[px] = LUT / float(NUM_SAMPLES);
}
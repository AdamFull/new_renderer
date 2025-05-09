#include "../include/bindles_layout.slang"
#include "unidirectional_shadow.h"

[[vk::push_constant]] ConstantBuffer<shadows::PushConstants> pc;

[[vk::binding(0, 1)]] ConstantBuffer<interop::EncodedGeometryData> egd : register(c0, space1);

struct PSInput {
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
    nointerpolation uint32_t material : TEXCOORD1;
};

struct PSOutput {
    float depth : SV_Depth;
};

[shader("fragment")]
#if defined(OMNI_SHADOW)
PSOutput 
#else
void
#endif
main(PSInput input) {
    // Alpha test on base-color texture (if set)
    if (input.material != ~0u) {
        interop::Material mat = g_fd.material_ptr[input.material];
        if (mat.base_color.valid()) {
            float alpha = sample_texture(mat.base_color, input.texcoord).a;
            if (alpha < mat.alpha_cutoff) {
                discard;
            }
        }
    }

#if defined(OMNI_SHADOW)
    // Compute normalized depth for omnidirectional shadow map
    interop::OmniShadowCommit shadow = g_fd.omni_shadow_ptr[pc.shadow_index];
    interop::LightCommit light  = g_fd.lights_ptr[shadow.index];

    float3 worldPos = input.pos.xyz;
    float dist = length(worldPos - light.position);
    float norm_depth= dist / shadow.far_plane;

    PSOutput output;
    output.depth = norm_depth;
    return output;
#endif
}
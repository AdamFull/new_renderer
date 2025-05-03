#include "../include/bindles_layout.slang"
#include "../include/compute_common.slang"
#include "../include/random.slang"
#include "../include/util.slang"
#include "apply_effect.h"

[[vk::binding(0, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(1, 1)]] SamplerState sampler0 : register(s0, space1);
[[vk::binding(2, 1)]] WTexture2D<float4> image0 : register(u0, space1);

#if defined(FXAA)
#include "../include/nvidia_fxaa.slang"
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantFXAA> pc;
#elif defined(MOTION_BLUR)
[[vk::binding(3, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(4, 1)]] SamplerState sampler1 : register(s1, space1);
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantMotionBlur> pc;
#elif defined(BLOOM)
[[vk::binding(3, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(4, 1)]] SamplerState sampler1 : register(s1, space1);
[[vk::binding(5, 1)]] Texture2D texture2 : register(t2, space1);
[[vk::binding(6, 1)]] SamplerState sampler2 : register(s2, space1);
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantBloom> pc;
#elif defined(DOF)
[[vk::binding(3, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(4, 1)]] SamplerState sampler1 : register(s1, space1);
[[vk::binding(5, 1)]] Texture2D texture2 : register(t2, space1);
[[vk::binding(6, 1)]] SamplerState sampler2 : register(s2, space1);
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantDoF> pc;
#elif defined(CHROMATIC_ABERRATION)
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantChromaticAberration> pc;
#elif defined(TONEMAP)
[[vk::binding(3, 1)]] RWTexture1D<float> image1 : register(u1, space1);
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantTonemap> pc;
#elif defined(GAUSSIAN_BLUR)
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantGaussianBlur> pc;
#elif defined(FILM_GRAIN)
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantFilmGrain> pc;
#elif defined(VIGNETTE)
[[vk::push_constant]] ConstantBuffer<apply_effect::PushConstantVignette> pc;
#endif

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

    [branch]
    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    float4 color = texture0.SampleLevel(sampler0, uv, 0.0f);
#if defined(FXAA)
    color.xyz = FxaaPixelShader(uv, texture0, sampler0, pc.quality, pc.threshold, pc.threshold_min).xyz;
#elif defined(MOTION_BLUR)
    float2 velocity = texture1.SampleLevel(sampler1, uv, 0.0f).xy;
    velocity = util::decode_velocity(velocity) * pc.scale;
    velocity = min(float2(1.0), velocity);

    if (length(velocity) >= pc.threshold) {
        float speed = length(velocity / texel_size);
        int n_samples = clamp(int(speed), 1, pc.max_samples);

        for (int i = 1; i < n_samples; ++i) {
            float2 offset = velocity * (float(i) / float(n_samples - 1) - 0.5);
            color += texture0.SampleLevel(sampler0, uv + offset, 0.0f);
        }
        color /= float(n_samples);
    }
#elif defined(BLOOM)
    const float dirt_mask_intensity = 1.0f;
    float3 dirt = texture2.SampleLevel(sampler2, float2(uv.x, 1.f - uv.y), 0.0f).xyz * dirt_mask_intensity;
    float3 bloom = texture1.SampleLevel(sampler1, uv, 0.0f).xyz;
    color.xyz = lerp(color.xyz, bloom + bloom * dirt, float3(pc.strength));
#elif defined(DOF)
    interop::Camera camera = g_fd.get_default_camera();

    float depth = texture2.SampleLevel(sampler2, uv, 0.0f).x;
    float linear_depth = util::linearize_depth(depth, camera.z_near, camera.z_far);

    float3 texture_blured = texture1.SampleLevel(sampler1, uv, 0.0f).xyz;

    float near_end = pc.near_field + pc.near_transition;
    float far_start = pc.far_field - pc.far_transition;

    float near_visibility = smoothstep(pc.near_field * pc.focus_point, near_end * pc.focus_point, linear_depth);
    float far_visibility = 1.0f - smoothstep(far_start * pc.focus_point, pc.far_field * pc.focus_point, linear_depth);

    float3 temp = lerp(texture_blured, color.rgb, near_visibility);
    color.xyz = lerp(texture_blured, temp, far_visibility);
#elif defined(CHROMATIC_ABERRATION)
    const float reci_num_iter_f = 1.0 / float(pc.iterations);

    float4 sumcol = float4(0.0);
    float4 sumw = float4(0.0);
    for (int i = 0; i < pc.iterations; ++i) {
        float t = float(i) * reci_num_iter_f;
        float4 w = util::spectrum_offset(t);
        sumw += w;
        sumcol += w * texture0.SampleLevel(sampler0, util::barrel_distortion(uv, 0.6 * pc.distortion * t), 0.0f);
    }

    color.xyz = (sumcol / sumw).xyz;
#elif defined(TONEMAP)
    float avg_lum = image1.Load(0).x; 

    float3 Yxy = util::convert_rgb_2_yxy(color.xyz);

    float lp = Yxy.x / (9.6 * avg_lum + 0.0001);

    if (pc.adaptive > 0) {
        Yxy.x = util::reinhard2(lp, pc.white_point);
    }
    else {
        Yxy.x = Yxy.x * pow(2.0, pc.exposure);
    }

    color.xyz = util::convert_yxy_2_rgb(Yxy);
    color.xyz = util::to_gamma(color.rgb, pc.gamma);
#elif defined(GAUSSIAN_BLUR)
#ifdef HORIZONTAL
    float2 direction = float2(pc.strength, 0.0);
#else
    float2 direction = float2(0.0, pc.strength);
#endif // HORIZONTAL

#if defined(BLUR_TYPE_5)
    {
        float2 off1 = float2(1.3333333333333333f) * direction;
        color += texture0.SampleLevel(sampler0, uv, 0.0f) * 0.29411764705882354 * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off1 / image_size), 0.0f) * 0.35294117647058826f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off1 / image_size), 0.0f) * 0.35294117647058826f * pc.scale;
    }
#elif defined(BLUR_TYPE_9)
    {
        float2 off1 = float2(1.3846153846f) * direction;
        float2 off2 = float2(3.2307692308f) * direction;
        color += texture0.SampleLevel(sampler0, uv, 0.0f) * 0.2270270270f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off1 / image_size), 0.0f) * 0.3162162162f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off1 / image_size), 0.0f) * 0.3162162162f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off2 / image_size), 0.0f) * 0.0702702703f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off2 / image_size), 0.0f) * 0.0702702703f * pc.scale;
    }
#elif defined(BLUR_TYPE_13)
    {
        float2 off1 = float2(1.411764705882353f) * direction;
        float2 off2 = float2(3.2941176470588234f) * direction;
        float2 off3 = float2(5.176470588235294f) * direction;
        color += texture0.SampleLevel(sampler0, uv, 0.0f) * 0.1964825501511404f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off1 / image_size), 0.0f) * 0.2969069646728344f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off1 / image_size), 0.0f) * 0.2969069646728344f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off2 / image_size), 0.0f) * 0.09447039785044732f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off2 / image_size), 0.0f) * 0.09447039785044732f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv + (off3 / image_size), 0.0f) * 0.010381362401148057f * pc.scale;
        color += texture0.SampleLevel(sampler0, uv - (off3 / image_size), 0.0f) * 0.010381362401148057f * pc.scale;
    }
#endif // BLUR_TYPE

#elif defined(FILM_GRAIN)
    let rng = random::RandomNumberGenerator(global_id.xy, image_size, g_fd.frame_number);
    color.xyz += pc.amount * rng.sample1d();
#elif defined(VIGNETTE)
    float3 tmp_color = color.rgb;
    tmp_color *= 1.0f - smoothstep(pc.inner_radius, pc.outer_radius, length(uv - 0.5f));
    color.rgb = lerp(color.rgb, tmp_color, pc.opacity);
#endif

    image0.Store(px, color);
}
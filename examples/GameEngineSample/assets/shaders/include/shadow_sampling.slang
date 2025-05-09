#pragma once

#include "random.slang"

namespace shadow {
static const float4x4 csm_bias_mat = float4x4(
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
);

static const float2 disk_offsets[16] = {
    float2(-0.94201624, -0.39906216),
    float2(0.94558609, -0.76890725),
    float2(-0.094184101, -0.92938870),
    float2(0.34495938, 0.29387760),
    float2(-0.91588581, 0.45771432),
    float2(-0.81544232, -0.87912464),
    float2(-0.38277543, 0.27676845),
    float2(0.97484398, 0.75648379),
    float2(0.44323325, -0.97511554),
    float2(0.53742981, -0.47373420),
    float2(-0.26496911, -0.41893023),
    float2(0.79197514, 0.19090188),
    float2(-0.24188840, 0.99706507),
    float2(-0.81409955, 0.91437590),
    float2(0.19984126, 0.78641367),
    float2(0.14383161, -0.14100790)
};

inline float cascaded_shadow_projection(Texture2DArray shadowmap_tex, SamplerComparisonState shadow_sampler, float4 shadow_coord, float2 offset, uint32_t cascade_index, float bias) {
    return shadowmap_tex.SampleCmpLevelZero(shadow_sampler, float3(shadow_coord.xy + offset, cascade_index), shadow_coord.z - bias);
}

inline float cascaded_shadow_filter_pcf(Texture2DArray shadowmap_tex, SamplerComparisonState shadow_sampler, float4 sc, uint32_t cascade_index, float bias) {
    uint32_t2 tex_dim;
    shadowmap_tex.GetDimensions(0, tex_dim.x, tex_dim.y);
    float2 texel_size = 1.0 / tex_dim;

    float shadow_factor = 0.0;

    // Random rotation and jitter
    const float rotation_angle = random::rand(sc.xy) * constants::two_pi;
    const float sin_angle = sin(rotation_angle);
    const float cos_angle = cos(rotation_angle);

    const float jitter = random::rand(sc.xy + float2(1.0, 1.0)) * 0.5; // Random jitter scale (0 to 0.5)

    [unroll]
    for (int i = 0; i < 16; ++i) {
        // Rotate the disk pattern
        float2 rotated_offset = float2(
            disk_offsets[i].x * cos_angle - disk_offsets[i].y * sin_angle,
                                       disk_offsets[i].x * sin_angle + disk_offsets[i].y * cos_angle
        );

        // Apply jitter to the offset
        rotated_offset += random::rand(sc.xy + float2(i, i)) * jitter;

        // Scale by texel size
        shadow_factor += cascaded_shadow_projection(shadowmap_tex, shadow_sampler, sc, rotated_offset * texel_size * 1.5, cascade_index, bias);
    }
    return shadow_factor * 0.0625;
}

//// For spotlight source
// inline float directional_shadow_projection(Texture2DArray shadowmap_tex, SamplerComparisonState shadow_sampler, float4 P, float2 offset, float bias, int layer) {
//     float4 shadow_coord = P / P.w;
//     shadow_coord.xy = shadow_coord.xy * 0.5 + 0.5;
//     return shadowmap_tex.SampleCmpLevelZero(shadow_sampler, float3(shadow_coord.xy + offset, layer), shadow_coord.z - bias);
// }
//
// inline float directionalShadowFilterPCF(Texture2DArray shadowmap_tex, SamplerComparisonState shadow_sampler, float4 sc, float bias, int layer) {
//     uint32_t2 tex_dim;
//     shadowmap_tex.GetDimensions(0, tex_dim.x, tex_dim.y);
//     float2 texel_size = 1.0 / tex_dim;
//
//     float shadow_factor = 0.0;
//
//     // Random rotation and jitter
//     const float rotation_angle = random::rand(sc.xy) * constants::two_pi;
//     const float sin_angle = sin(rotation_angle);
//     const float cos_angle = cos(rotation_angle);
//
//     const float jitter = random::rand(sc.xy + float2(1.0, 1.0)) * 0.5; // Random jitter scale (0 to 0.5)
//
//     [unroll]
//     for (int i = 0; i < 16; ++i) {
//         // Rotate the disk pattern
//         float2 rotated_offset = float2(
//             disk_offsets[i].x * cos_angle - disk_offsets[i].y * sin_angle,
//                                        disk_offsets[i].x * sin_angle + disk_offsets[i].y * cos_angle
//         );
//
//         // Apply jitter to the offset
//         rotated_offset += random::rand(sc.xy + float2(i, i)) * jitter;
//         shadow_factor += directional_shadow_projection(shadowmap_tex, shadow_sampler, sc, rotated_offset * texel_size * 1.5, bias, layer);
//     }
//     return shadow_factor * 0.0625;
// }
//
//// For omni directional source
//// Based on https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
//
// static const float3 sampleOffsetDirections[20] = float3[](
//   float3(1, 1, 1),  float3(1, -1, 1),  float3(-1, -1, 1),  float3(-1, 1, 1),
//   float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
//   float3(1, 1, 0),  float3(1, -1, 0),  float3(-1, -1, 0),  float3(-1, 1, 0),
//   float3(1, 0, 1),  float3(-1, 0, 1),  float3(1, 0, -1),   float3(-1, 0, -1),
//   float3(0, 1, 1),  float3(0, -1, 1),  float3(0, -1, -1),  float3(0, 1, -1)
//);
//
// inline float omni_shadow_projection(TextureCubeArray shadowmap_tex, SamplerComparisonState shadow_sampler, float3 shadow_clip, float3 offset, float bias, float radius, int layer) {
//    float current_dist = length(shadow_clip) / radius;
//    return shadowmap_tex.SampleCmpLevelZero(shadow_sampler, float4(shadow_clip + offset, layer), current_dist - bias);
//}
//
// inline float omni_shadow_filter_pcf(TextureCubeArray shadowmap_tex, SamplerComparisonState shadow_sampler, float3 shadow_clip, float bias, float radius, int layer) {
//    float shadowFactor = 0.0;
//    float samples = 4.0;
//    const float offset = 0.1;
//
//    for (float x = -offset; x < offset; x += offset / (samples * 0.5)) {
//        for (float y = -offset; y < offset; y += offset / (samples * 0.5)) {
//            for (float z = -offset; z < offset; z += offset / (samples * 0.5))
//                shadowFactor += omni_shadow_projection(shadowmap_tex, shadow_sampler, shadow_clip, float3(x, y, z), bias, radius, layer);
//        }
//    }
//    shadowFactor /= (samples * samples * samples);
//    return shadowFactor;
//}
}
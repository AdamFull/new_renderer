#pragma once

#include "constants.h"

struct MaterialSample {
    float3 albedo;
    float opacity;

    float3 emission;

    float metallic;
    float roughness;

    float3 normal;

    float ao;

    float ior;
};

inline float3 get_cube_side_direction(in float2 uv, in uint32_t side) {
    const float2 tex = uv * 2.0f - 1.0f;

    [branch]
    switch (side) {
    case 0: return float3(1.0f, -tex.y, -tex.x); // Front
    case 1: return float3(-1.0f, -tex.y, tex.x); // Back
    case 2: return float3(tex.x, 1.0f, tex.y); // Right
    case 3: return float3(tex.x, -1.0f, -tex.y); // Left
    case 4: return float3(tex.x, -tex.y, 1.0f); // Top
    case 5: return float3(-tex.x, -tex.y, -1.0f); // Bottom
    default: return float3(1.0);
    }
}

// Normal Distribution function --------------------------------------
inline float d_ggx(in float dotNH, in float roughness) {
    const float alpha = roughness * roughness;
    const float alpha2 = alpha * alpha;
    const float denom = dotNH * dotNH * (alpha2 - 1.0f) + 1.0f;
    return (alpha2) / (constants::pi * denom * denom);
}

// Geometric Shadowing function --------------------------------------
inline float g_schlicksmith_ggx(in float dot_nl, in float dot_nv, in float roughness) {
    const float r = (roughness + 1.0f);
    const float k = (r * r) / 8.0f;
    const float GL = dot_nl / (dot_nl * (1.0f - k) + k);
    const float GV = dot_nv / (dot_nv * (1.0f - k) + k);
    return GL * GV;
}

inline float3 prefiltered_reflection(in float3 R, in float roughness, in TextureCube<float4> prefiltered, in SamplerState cube_sampler) {
    uint width, height, numLevels;
    prefiltered.GetDimensions(0, width, height, numLevels);

    const float lod = roughness * numLevels;
    const float lodf = floor(lod);
    const float lodc = ceil(lod);
    float3 a = pow(prefiltered.SampleLevel(cube_sampler, R, lodf).rgb, 2.2f);
    float3 b = pow(prefiltered.SampleLevel(cube_sampler, R, lodc).rgb, 2.2f);
    return lerp(a, b, lod - lodf);
}

// Fresnel function ----------------------------------------------------
inline float3 f0_from_ior(in float ior) {
    const float a = (1.f - ior) / (1.f + ior);
    return float3(a * a);
}

inline float3 f_schlick(in float cosTheta, in float f90, in float3 F0) {
    return F0 + (f90 - F0) * pow(1.0f - cosTheta, 5.0f);
}

inline float3 f_schlick_r(in float cosTheta, float3 F0, in float roughness) {
    return F0 + (max(float3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

inline float3 specular_contribution(in float3 diffuse, in float3 L, in float3 V, in float3 N, in float3 F0, in float metallic, in float roughness) {
    // Precalculate vectors and dot products
    const float3 H = normalize(V + L);
    const float dotNH = clamp(dot(N, H), 0.0, 1.0);
    const float dotNV = clamp(dot(N, V), 0.0, 1.0);
    const float dotNL = clamp(dot(N, L), 0.0, 1.0);

    float3 color = float3(0.0f);

    if (dotNL > 0.0f) {
        // D = Normal distribution (Distribution of the microfacets)
        const float D = d_ggx(dotNH, roughness);
        // G = Geometric shadowing term (Microfacets shadowing)
        const float G = g_schlicksmith_ggx(dotNL, dotNV, roughness);
        // F = Fresnel factor (Reflectance depending on angle of incidence)
        const float3 F = f_schlick(dotNV, 1.0, F0);
        const float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        const float3 kD = (float3(1.0f) - F) * (1.0f - metallic);
        color += (kD * diffuse / constants::pi + spec) * dotNL;
    }

    return color;
}
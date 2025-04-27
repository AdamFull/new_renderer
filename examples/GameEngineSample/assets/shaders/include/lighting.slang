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

// Fresnel function ----------------------------------------------------
float3 f0_from_ior(in float ior) {
    const float a = (1.f - ior) / (1.f + ior);
    return float3(a * a);
}

float3 f_schlick(in float cosTheta, in float f90, in float3 F0) {
    return F0 + (f90 - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 f_schlick_r(in float cosTheta, float3 F0, in float roughness) {
    return F0 + (max(float3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}
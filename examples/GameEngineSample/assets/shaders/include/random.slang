#pragma once

#include "constants.h"

inline uint32_t flatten_2d(in uint32_t2 index, in uint32_t dimensions_x) {
    return index.x + index.y * dimensions_x;
}

inline uint32_t2 unflatten_2d(in uint32_t index, in uint32_t dimensions_x) {
    return uint32_t2(index % dimensions_x, index / dimensions_x);
}

inline uint32_t flatten_3d(uint32_t3 index, uint32_t2 dimensions_xy) {
    return index.x + index.y * dimensions_xy.x + index.z * dimensions_xy.x * dimensions_xy.y;
}

uint32_t3 unflatten_3d(uint32_t index, uint32_t2 dimensions_xy) {
    uint32_t3 out_index;
    out_index.z = index / (dimensions_xy.x * dimensions_xy.y);
    index -= (out_index.z * dimensions_xy.x * dimensions_xy.y);
    out_index.y = index / dimensions_xy.x;
    out_index.x = index % dimensions_xy.x;
    return out_index;
}

namespace random {
    inline float rand(float2 co) {
        const float a = 12.9898f;
        const float b = 78.233f;
        const float c = 43758.5453f;
        const float dt = dot(co.xy, float2(a, b));
        const float sn = fmod(dt, constants::pi);
        return fract(sin(sn) * c);
    }

    inline uint32_t hash(in uint32_t seed) {
        const uint32_t state = seed * 747796405u + 2891336453u;
        const uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    inline uint32_t xor_shift(inout uint32_t rng_state) {
        // Xorshift algorithm from George Marsaglia's paper.
        rng_state ^= (rng_state << 13);
        rng_state ^= (rng_state >> 17);
        rng_state ^= (rng_state << 5);
        return rng_state;
    }

    float2 hammersley2d(in uint32_t i, in uint32_t N) {
        // Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
        uint bits = (i << 16u) | (i >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        const float rdi = float(bits) * 2.3283064365386963e-10;
        return float2(float(i) / float(N), rdi);
    }

    float3 hemispherepoint_uniform(in float u, in float v) {
        const float phi = v * constants::two_pi;
        const float cosTheta = 1.0 - u;
        const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    }

    // Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
    float3 importance_sample_ggx(in float2 Xi, in float roughness, in float3 normal) {
        // Maps a 2D point to a hemisphere with spread based on roughness
        const float alpha = roughness * roughness;
        const float phi = constants::two_pi * Xi.x + rand(normal.xz) * 0.1f;
        const float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (alpha * alpha - 1.0f) * Xi.y));
        const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
        const float3 H = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

        // Tangent space
        const float3 up = abs(normal.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
        const float3 tangentX = normalize(cross(up, normal));
        const float3 tangentY = normalize(cross(normal, tangentX));

        // Convert to world Space
        return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
    }
    
    struct RandomNumberGenerator {
        __init(uint32_t2 pixel, uint32_t2 resolution, uint32_t frame_index) {
            rng_state = flatten_2d(pixel, resolution.x) ^ hash(frame_index);
            rng_state = hash(rng_state);
        }

        __init(uint32_t initial_hash) {
            rng_state = hash(initial_hash);
        }

        [mutating]
        inline float sample() {
            return asfloat(0x3f800000 | xor_shift(rng_state) >> 9) - 1.0;
        }

        inline float sample1d() {
            return sample();
        }

        inline float2 sample_2d() {
            return float2(sample(), sample());
        }

        inline float3 sample_3d() {
            return float3(sample(), sample(), sample());
        }

        inline float4 sample_4d() {
            return float4(sample(), sample(), sample(), sample());
        }

        uint32_t rng_state;
    };
}
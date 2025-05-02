#pragma once

#include "interop.h"

[[vk::binding(FRAME_DATA_SLOT, 0)]] ConstantBuffer<interop::FrameData> g_fd : register(c0, space0);
[[vk::binding(SAMPLER_SLOT, 0)]] SamplerState g_samplers[MAX_SAMPLER_SLOTS] : register(s0, space0);
[[vk::binding(SRV_TEXTURE_SLOT, 0)]] Texture2D g_srv_textures[MAX_SRV_TEXTURE_SLOTS] : register(t0, space0);
[[vk::binding(UAV_TEXTURE_SLOT, 0)]] WTexture2D<float4> g_uav_textures[MAX_UAV_TEXTURE_SLOTS] : register(u0, space0);

float4 sample_texture(in interop::Texture h, float2 uv) {
    Texture2D texture = g_srv_textures[NonUniformResourceIndex(h.texture)];
    SamplerState sampler = g_samplers[NonUniformResourceIndex(h.sampler)];
    return texture.SampleLevel(sampler, uv, 0.0f);
}

float4 sample_texture_grad(in interop::Texture h, float2 uv, float2 uv_dx, float2 uv_dy) {
    Texture2D texture = g_srv_textures[NonUniformResourceIndex(h.texture)];
    SamplerState sampler = g_samplers[NonUniformResourceIndex(h.sampler)];
    return texture.SampleGrad(sampler, uv, uv_dx, uv_dy);
}
#pragma once

#include "interop.h"

[[vk::binding(0, FRAME_DATA_SLOT)]] ConstantBuffer<interop::FrameData> g_fd : register(c0, space0);
[[vk::binding(0, SAMPLER_SLOT)]] SamplerState g_samplers[MAX_SAMPLER_SLOTS] : register(s0, space0);
[[vk::binding(0, SRV_TEXTURE_SLOT)]] Texture2D g_srv_textures[MAX_SRV_TEXTURE_SLOTS] : register(t0, space0);
[[vk::binding(0, UAV_TEXTURE_SLOT)]] WTexture2D<float4> g_uav_textures[MAX_UAV_TEXTURE_SLOTS] : register(u0, space0);

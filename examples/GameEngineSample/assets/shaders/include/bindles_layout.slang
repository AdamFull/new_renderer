#pragma once

#include "interop.h"

[[vk::binding(FRAME_DATA_SLOT, 0)]] ConstantBuffer<interop::FrameData> g_fd : register(c0, space0);
[[vk::binding(SAMPLER_SLOT, 0)]] SamplerState g_samplers[MAX_SAMPLER_SLOTS] : register(s0, space0);
[[vk::binding(SRV_TEXTURE_SLOT, 0)]] Texture2D g_srv_textures[MAX_SRV_TEXTURE_SLOTS] : register(t0, space0);
[[vk::binding(UAV_TEXTURE_SLOT, 0)]] WTexture2D<float4> g_uav_textures[MAX_UAV_TEXTURE_SLOTS] : register(u0, space0);

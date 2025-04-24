#ifndef SHLIB_BINDLESS_H
#define SHLIB_BINDLESS_H

#include "base.h"

SHLIB_NS_BEGIN
CONSTANT uint32_t MAX_SAMPLERS = 2048u;
CONSTANT uint32_t MAX_SRV_TEXTURES = 2048u;
CONSTANT uint32_t MAX_UAV_TEXTURES = 512u;

CONSTANT uint32_t SAMPLER_SLOT = 0;
CONSTANT uint32_t SRV_TEXTURE_SLOT = 1;
CONSTANT uint32_t UAV_TEXTURE_SLOT = 2;

struct SamplerState { uint32_t s; };

struct Texture1D { uint32_t t; };
struct Texture2D { uint32_t t; };
struct Texture3D { uint32_t t; };
struct TextureCube { uint32_t t; };

struct Image1D { uint32_t i; };
struct Image2D { uint32_t i; };
struct Image3D { uint32_t i; };
struct ImageCube { uint32_t i; };

SHLIB_NS_END

#ifndef __cplusplus
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = SAMPLER_SLOT) uniform sampler g_samplers[MAX_SAMPLERS];

layout(binding = MAX_SRV_TEXTURES) uniform texture1D g_srv_textures_1d[MAX_SRV_TEXTURES];
#define GET_SAMPLER(s, t) sampler1D(g_srv_textures_1d[t.t], g_samplers[s.s])
vec4 texture(SamplerState s, Texture1D t, float uv) {
	return texture(GET_SAMPLER(s, t), uv);
}

vec4 texture_lod(SamplerState s, Texture1D t, float uv, float lod) {
	return textureLod(GET_SAMPLER(s, t), uv, lod);
}

vec4 texel_fetch(Texture1D t, int pos, int lod) {
	return texelFetch(g_srv_textures_1d[t.t], pos, lod);
}

int texture_size(Texture1D t, int lod) { 
	return textureSize(g_srv_textures_1d[t.t], lod);
}
#undef GET_SAMPLER

layout(binding = MAX_SRV_TEXTURES) uniform texture2D g_srv_textures_2d[MAX_SRV_TEXTURES];
#define GET_SAMPLER(s, t) sampler2D(g_srv_textures_2d[t.t], g_samplers[s.s])
vec4 texture(SamplerState s, Texture2D t, vec2 uv) {
	return texture(GET_SAMPLER(s, t), uv);
}

vec4 texture_lod(SamplerState s, Texture2D t, vec2 uv, float lod) {
	return textureLod(GET_SAMPLER(s, t), uv, lod);
}

vec4 texel_fetch(Texture2D t, ivec2 pos, int lod) {
	return texelFetch(g_srv_textures_2d[t.t], pos, lod);
}

ivec2 texture_size(Texture2D t, int lod) {
	return textureSize(g_srv_textures_2d[t.t], lod);
}
#undef GET_SAMPLER

layout(binding = MAX_SRV_TEXTURES) uniform texture3D g_srv_textures_3d[MAX_SRV_TEXTURES];
#define GET_SAMPLER(s, t) sampler3D(g_srv_textures_3d[t.t], g_samplers[s.s])
vec4 texture(SamplerState s, Texture3D t, vec3 uv) {
	return texture(GET_SAMPLER(s, t), uv);
}

vec4 texture_lod(SamplerState s, Texture3D t, vec3 uv, float lod) {
	return textureLod(GET_SAMPLER(s, t), uv, lod);
}

vec4 texel_fetch(Texture3D t, ivec3 pos, int lod) {
	return texelFetch(g_srv_textures_2d[t.t], pos, lod);
}

ivec3 texture_size(Texture3D t, int lod) {
	return textureSize(g_srv_textures_3d[t.t], lod);
}
#undef GET_SAMPLER

layout(binding = MAX_SRV_TEXTURES) uniform textureCube g_srv_textures_cube[MAX_SRV_TEXTURES];
#define GET_SAMPLER(s, t) samplerCube(g_srv_textures_cube[t.t], g_samplers[s.s])
vec4 texture(SamplerState s, TextureCube t, vec3 uv) {
	return texture(GET_SAMPLER(s, t), uv);
}

vec4 texture_lod(SamplerState s, TextureCube t, vec3 uv, float lod) {
	return textureLod(GET_SAMPLER(s, t), uv, lod);
}

vec4 texel_fetch(TextureCube t, ivec3 pos, int lod) {
	return texelFetch(g_srv_textures_cube[t.t], pos, lod);
}

ivec3 texture_size(TextureCube t, int lod) {
	return textureSize(g_srv_textures_cube[t.t], lod);
}
#undef GET_SAMPLER

layout(binding = UAV_TEXTURE_SLOT) restrict uniform image1D  g_uav_textures_1d[MAX_UAV_TEXTURES];
int image_size(Image1D i) { 
	return imageSize(g_uav_textures_1d[i.i]);
}

vec4 image_load(Image1D i, int pos) {
	return imageLoad(g_uav_textures_1d[i.i], pos);
}

void image_store(Image1D i, int pos, vec4 data) {
	imageStore(g_uav_textures_1d[i.i], pos, data);
}

layout(binding = UAV_TEXTURE_SLOT) restrict uniform image1D  g_uav_textures_2d[MAX_UAV_TEXTURES];
int image_size(Image1D i) {
	return imageSize(g_uav_textures_1d[i.i]);
}

vec4 image_load(Image1D i, int pos) {
	return imageLoad(g_uav_textures_1d[i.i], pos);
}

void image_store(Image1D i, int pos, vec4 data) {
	imageStore(g_uav_textures_1d[i.i], pos, data);
}

layout(binding = UAV_TEXTURE_SLOT) restrict uniform image1D  g_uav_textures_3d[MAX_UAV_TEXTURES];
layout(binding = UAV_TEXTURE_SLOT) restrict uniform image1D  g_uav_textures_cube[MAX_UAV_TEXTURES];

#endif // ifndef __cplusplus

#endif
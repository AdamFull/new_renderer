#ifndef SHLIB_GEOMETRY_GLSL
#define SHLIB_GEOMETRY_GLSL

#include "shlib.h"

#ifdef GEOMETRY_BUFFERS

layout(std140, set = 1, binding = 0) uniform GeometryBuffers
{
	GET_PTR_T(float) position;
	GET_PTR_T(float) color;
	GET_PTR_T(float) texcoord;
	GET_PTR_T(uint16_t) normal;
	GET_PTR_T(uint16_t) tangent;
#if defined(JOINTS_16BIT)
	GET_PTR_T(uint16_t) joint;
#else
	GET_PTR_T(uint8_t) joint;
#endif
	GET_PTR_T(float) weight;

#if defined(INDEX_16BIT)
	GET_PTR_T(uint16_t) indices;
#else
	GET_PTR_T(uint32_t) indices;
#endif
} geometry_buffers;

vec3 GetVertexPosition(uint32_t vertex_index)
{
	return vec3(
	geometry_buffers.position[vertex_index * 3 + 0].data,
	geometry_buffers.position[vertex_index * 3 + 1].data,
	geometry_buffers.position[vertex_index * 3 + 2].data);
}

vec4 GetVertexColor(uint32_t vertex_index)
{
#ifdef HAS_COLOR
	u8vec4 encoded_color = u8vec4(
		geometry_buffers.color[vertex_index * 4 + 0].data,
		geometry_buffers.color[vertex_index * 4 + 1].data,
		geometry_buffers.color[vertex_index * 4 + 2].data,
		geometry_buffers.color[vertex_index * 4 + 3].data
	);

	return DecodeColor(encoded_color);
#else
	return vec4(0.f);
#endif
}

vec2 GetVertexTexcoord(uint32_t vertex_index)
{
#ifdef HAS_TEXCOORD
	return vec2(
	geometry_buffers.texcoord[vertex_index * 2 + 0].data,
	geometry_buffers.texcoord[vertex_index * 2 + 1].data);
#else
	return vec2(0.f, 0.f);
#endif
}

vec3 GetVertexNormal(uint32_t vertex_index)
{
#ifdef HAS_NORMAL
	u16vec2 encoded_normal = u16vec2(
		geometry_buffers.normal[vertex_index * 2 + 0].data,
		geometry_buffers.normal[vertex_index * 2 + 1].data);

	return DecodeNormal(encoded_normal);
#else
	return vec3(0.f);
#endif
}

vec4 GetVertexTangent(uint32_t vertex_index)
{
#ifdef HAS_TANGENT
	u16vec2 encoded_tangent = u16vec2(
		geometry_buffers.tangent[vertex_index * 2 + 0].data,
		geometry_buffers.tangent[vertex_index * 2 + 1].data);

	return DecodeTangent(encoded_tangent);
#else
	return vec4(0.f);
#endif
}

Joints GetVertexJoints(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return Joints(
	geometry_buffers.joint[vertex_index * 4 + 0].data,
	geometry_buffers.joint[vertex_index * 4 + 1].data,
	geometry_buffers.joint[vertex_index * 4 + 2].data,
	geometry_buffers.joint[vertex_index * 4 + 3].data
	);
#else
	return Joints(0);
#endif
}

vec4 GetVertexWeights(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return vec4(
	geometry_buffers.weight[vertex_index * 4 + 0].data,
	geometry_buffers.weight[vertex_index * 4 + 1].data,
	geometry_buffers.weight[vertex_index * 4 + 2].data,
	geometry_buffers.weight[vertex_index * 4 + 3].data
	);
#else
	return vec4(0.f);
#endif
}

Index GetIndex(uint32_t offset)
{
	return geometry_buffers.indices[offset].data;
}

struct FRawVertex
{
	vec3 position;
	vec4 color;

	vec2 uv;
	vec3 normal;

	vec4 tangent;
	
	Joints joints;
	vec4 weights;
};

#endif // GEOMETRY_BUFFERS

#endif // SHLIB_GEOMETRY_GLSL
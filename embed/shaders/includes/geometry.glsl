#ifndef SHLIB_GEOMETRY_GLSL
#define SHLIB_GEOMETRY_GLSL

#include "shlib.h"

#ifdef GEOMETRY_BUFFERS

layout(std140, set = 1, binding = 0) uniform GeometryBuffers
{
	GET_PTR_T(i16vec3) position;
	GET_PTR_T(u8vec4) color;
	GET_PTR_T(u16vec2) texcoord;
	GET_PTR_T(u16vec2) normal;
	GET_PTR_T(u16vec2) tangent;
#if defined(JOINTS_16BIT)
	GET_PTR_T(u16vec4) joints;
#else
	GET_PTR_T(u8vec4) joints;
#endif
	GET_PTR_T(vec3) weights;

#if defined(INDEX_16BIT)
	GET_PTR_T(uint16_t) indices;
#else
	GET_PTR_T(uint32_t) indices;
#endif
} geometry_buffers;

vec3 GetVertexPosition(uint32_t vertex_index)
{
	return DecodePosition(geometry_buffers.position[vertex_index].data);
}

vec4 GetVertexColor(uint32_t vertex_index)
{
#ifdef HAS_COLOR
	return DecodeColor(geometry_buffers.color[vertex_index].data);
#else
	return vec4(0.f);
#endif
}

vec2 GetVertexTexcoord(uint32_t vertex_index, TexcoordBounds bounds)
{
#ifdef HAS_TEXCOORD
	return DecodeTexcoord(geometry_buffers.texcoord[vertex_index].data, bounds);
	return geometry_buffers.texcoord[vertex_index].data;
#else
	return vec2(0.f, 0.f);
#endif
}

vec3 GetVertexNormal(uint32_t vertex_index)
{
#ifdef HAS_NORMAL
	return DecodeNormal(geometry_buffers.normal[vertex_index].data);
#else
	return vec3(0.f);
#endif
}

vec4 GetVertexTangent(uint32_t vertex_index)
{
#ifdef HAS_TANGENT
	return DecodeTangent(geometry_buffers.tangent[vertex_index].data);
#else
	return vec4(0.f);
#endif
}

Joints GetVertexJoints(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return geometry_buffers.joints[vertex_index].data;
#else
	return Joints(0);
#endif
}

vec4 GetVertexWeights(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	vec3 weight = geometry_buffers.weights[vertex_index].data;
	return vec4(weight, 1.0f - weight.x - weight.y - weight.z);
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
#ifndef SHLIB_GEOMETRY_GLSL
#define SHLIB_GEOMETRY_GLSL

#include "shlib.h"

#ifdef GEOMETRY_BUFFERS

layout(std140, set = 1, binding = 0) uniform GeometryBuffers
{
	GET_ARR_PTR_T(float) position;
	GET_ARR_PTR_T(float) color;
	GET_ARR_PTR_T(float) texcoord;
	GET_ARR_PTR_T(float) normal;
	GET_ARR_PTR_T(float) tangent;
#if defined(JOINTS_16BIT)
	GET_ARR_PTR_T(uint16_t) joint;
#else
	GET_ARR_PTR_T(uint8_t) joint;
#endif
	GET_ARR_PTR_T(float) weight;

#if defined(INDEX_16BIT)
	GET_ARR_PTR_T(uint16_t) indices;
#else
	GET_ARR_PTR_T(uint32_t) indices;
#endif
} geometry_buffers;

vec3 GetVertexPosition(uint32_t vertex_index)
{
	return vec3(
	PTRVAL(geometry_buffers.position, vertex_index * 3 + 0),
	PTRVAL(geometry_buffers.position, vertex_index * 3 + 1),
	PTRVAL(geometry_buffers.position, vertex_index * 3 + 2));
}

Color GetVertexColor(uint32_t vertex_index)
{
#if HAS_COLOR
	return Color(
	PTRVAL(geometry_buffers.color, vertex_index * VTX_COLOR_ELEM_CNT + 0)
	,PTRVAL(geometry_buffers.color, vertex_index * VTX_COLOR_ELEM_CNT + 1)
	,PTRVAL(geometry_buffers.color, vertex_index * VTX_COLOR_ELEM_CNT + 2)
#if defined(COLOR_VEC4)
	,PTRVAL(geometry_buffers.color, vertex_index * VTX_COLOR_ELEM_CNT + 3)
#endif
	);
#else
	return Color(0.f);
#endif
}

vec2 GetVertexTexcoord(uint32_t vertex_index)
{
#ifdef HAS_TEXCOORD
	return vec2(
	PTRVAL(geometry_buffers.texcoord, vertex_index * 2 + 0),
	PTRVAL(geometry_buffers.texcoord, vertex_index * 2 + 1));
#else
	return vec2(0.f, 0.f);
#endif
}

vec3 GetVertexNormal(uint32_t vertex_index)
{
#ifdef HAS_NORMAL
	return vec3(
	PTRVAL(geometry_buffers.normal, vertex_index * 3 + 0),
	PTRVAL(geometry_buffers.normal, vertex_index * 3 + 1),
	PTRVAL(geometry_buffers.normal, vertex_index * 3 + 2));
#else
	return vec3(0.f);
#endif
}

Tangent GetVertexTangent(uint32_t vertex_index)
{
#if HAS_TANGENT
	return Tangent(
	PTRVAL(geometry_buffers.tangent, vertex_index * VTX_TANGENT_ELEM_CNT + 0)
	,PTRVAL(geometry_buffers.tangent, vertex_index * VTX_TANGENT_ELEM_CNT + 1)
	,PTRVAL(geometry_buffers.tangent, vertex_index * VTX_TANGENT_ELEM_CNT + 2)
#if defined(TANGENT_VEC4)
	,PTRVAL(geometry_buffers.tangent, vertex_index * VTX_TANGENT_ELEM_CNT + 3)
#endif
	);
#else
	return Tangent(0.f);
#endif
}

Joints GetVertexJoints(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return Joints(
	PTRVAL(geometry_buffers.joint, vertex_index * 4 + 0)
	,PTRVAL(geometry_buffers.joint, vertex_index * 4 + 1)
	,PTRVAL(geometry_buffers.joint, vertex_index * 4 + 2)
	,PTRVAL(geometry_buffers.joint, vertex_index * 4 + 3)
	);
#else
	return Joints(0);
#endif
}

vec4 GetVertexWeights(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return vec4(
	PTRVAL(geometry_buffers.weight, vertex_index * 4 + 0)
	,PTRVAL(geometry_buffers.weight, vertex_index * 4 + 1)
	,PTRVAL(geometry_buffers.weight, vertex_index * 4 + 2)
	,PTRVAL(geometry_buffers.weight, vertex_index * 4 + 3)
	);
#else
	return vec4(0.f);
#endif
}

Index GetIndex(uint32_t offset)
{
	return PTRVAL(geometry_buffers.indices, offset);
}

struct FRawVertex
{
	vec3 position;
	Color color;

	vec2 uv;
	vec3 normal;

	Tangent tangent;
	
	Joints joints;
	vec4 weights;
};

#endif // GEOMETRY_BUFFERS

#endif // SHLIB_GEOMETRY_GLSL
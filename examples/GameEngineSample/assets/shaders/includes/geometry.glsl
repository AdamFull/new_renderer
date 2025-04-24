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
	GET_PTR_T(u16vec3) weights;

#if defined(INDEX_16BIT)
	GET_PTR_T(uint16_t) indices;
#else
	GET_PTR_T(uint32_t) indices;
#endif
} geometry_buffers;

layout(std140, set = 1, binding = 16) uniform DrawDescriptionBuffers
{
	GET_PTR_T(DrawDescription) draw_descriptions_ptr;
	GET_PTR_T(DrawIndexedIndirectData) draw_commands_ptr;
};

DrawDescription GetDrawInfo(uint32_t index)
{
	return draw_descriptions_ptr[index].data;
}

DrawIndexedIndirectData GetDrawIndexedCommand(uint32_t index)
{
	return draw_commands_ptr[index].data;
}

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

joints_t GetVertexJoints(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return geometry_buffers.joints[vertex_index].data;
#else
	return joints_t(0);
#endif
}

vec4 GetVertexWeights(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	return DecodeWeights(geometry_buffers.weights[vertex_index].data);
#else
	return vec4(0.f);
#endif
}

sindex_t GetIndex(uint32_t offset)
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
	
	joints_t joints;
	vec4 weights;
};

#endif // GEOMETRY_BUFFERS

#endif // SHLIB_GEOMETRY_GLSL
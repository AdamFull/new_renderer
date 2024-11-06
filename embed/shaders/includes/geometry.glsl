#ifndef GEOMETRY_GLSL
#define GEOMETRY_GLSL

#include "interop/base"
#include "buffer_pointer"

// Define vertex color type dependent on element count
#if (defined(COLOR_VEC3) || defined(COLOR_VEC4))
#define HAS_COLOR 1
#else
#define HAS_COLOR 0
#endif

#if defined(COLOR_VEC4)
#define Color vec4
#define VTX_COLOR_ELEM_CNT 4
#else
#define Color vec3
#define VTX_COLOR_ELEM_CNT 3
#endif

// Define vertex tangent type also dependent on element count
#if (defined(TANGENT_VEC3) || defined(TANGENT_VEC4))
#define HAS_TANGENT 1
#else
#define HAS_TANGENT 0
#endif

#if defined(TANGENT_VEC4)
#define Tangent vec4
#define VTX_TANGENT_ELEM_CNT 4
#else // TANGENT_VEC4
#define Tangent vec3
#define VTX_TANGENT_ELEM_CNT 3
#endif // TANGENT_VEC3

// Define joints type
#if defined(JOINTS_16BIT)
#define JointIndex uint16_t
#define Joints u16vec4
#define JOINT_ALIGN 2
#else
#define JointIndex uint8_t
#define Joints u8vec4
#define JOINT_ALIGN 1
#endif

#if defined(INDEX_16BIT)
#define INDEX_ALIGN 2
#else
#define INDEX_ALIGN 4
#endif

#ifdef GEOMETRY_BUFFERS

// Register buffer pointers
REGISTER_RO_BUFFER_PTR(PositionBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(ColorBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(TexcoordBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(NormalBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(TangentBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(JointBuffer, JointIndex, JOINT_ALIGN);
REGISTER_RO_BUFFER_PTR(WeightBuffer, float, 4);
REGISTER_RO_BUFFER_PTR(IndexBuffer, Index, INDEX_ALIGN);

layout(std140, set = 1, binding = 0) uniform GeometryBuffers
{
	uint64_t position;
	uint64_t color;
	uint64_t texcoord;
	uint64_t normal;
	uint64_t tangent;
	uint64_t joint;
	uint64_t weight;

	uint64_t indices;
} geometry_buffers;

vec3 GetVertexPosition(uint32_t vertex_index)
{
	PositionBuffer position_buffer = PTRCAST(PositionBuffer, geometry_buffers.position);

	return vec3(
	PTRVAL(position_buffer, vertex_index * 3 + 0),
	PTRVAL(position_buffer, vertex_index * 3 + 1),
	PTRVAL(position_buffer, vertex_index * 3 + 2));
}

Color GetVertexColor(uint32_t vertex_index)
{
#if HAS_COLOR
	ColorBuffer color_buffer = PTRCAST(ColorBuffer, geometry_buffers.color);

	return Color(
	PTRVAL(color_buffer, vertex_index * VTX_COLOR_ELEM_CNT + 0)
	,PTRVAL(color_buffer, vertex_index * VTX_COLOR_ELEM_CNT + 1)
	,PTRVAL(color_buffer, vertex_index * VTX_COLOR_ELEM_CNT + 2)
#if defined(COLOR_VEC4)
	,PTRVAL(color_buffer, vertex_index * VTX_COLOR_ELEM_CNT + 3)
#endif
	);
#else
	return Color(0.f);
#endif
}

vec2 GetVertexTexcoord(uint32_t vertex_index)
{
#ifdef HAS_TEXCOORD
	TexcoordBuffer texcoord_buffer = PTRCAST(TexcoordBuffer, geometry_buffers.texcoord);

	return vec2(
	PTRVAL(texcoord_buffer, vertex_index * 2 + 0),
	PTRVAL(texcoord_buffer, vertex_index * 2 + 1));
#else
	return vec2(0.f, 0.f);
#endif
}

vec3 GetVertexNormal(uint32_t vertex_index)
{
#ifdef HAS_NORMAL
	NormalBuffer normal_buffer = PTRCAST(NormalBuffer, geometry_buffers.normal);

	return vec3(
	PTRVAL(normal_buffer, vertex_index * 3 + 0),
	PTRVAL(normal_buffer, vertex_index * 3 + 1),
	PTRVAL(normal_buffer, vertex_index * 3 + 2));
#else
	return vec3(0.f);
#endif
}

Tangent GetVertexTangent(uint32_t vertex_index)
{
#if HAS_TANGENT
	TangentBuffer tangent_buffer = PTRCAST(TangentBuffer, geometry_buffers.tangent);

	return Tangent(
	PTRVAL(tangent_buffer, vertex_index * VTX_TANGENT_ELEM_CNT + 0)
	,PTRVAL(tangent_buffer, vertex_index * VTX_TANGENT_ELEM_CNT + 1)
	,PTRVAL(tangent_buffer, vertex_index * VTX_TANGENT_ELEM_CNT + 2)
#if defined(TANGENT_VEC4)
	,PTRVAL(tangent_buffer, vertex_index * VTX_TANGENT_ELEM_CNT + 3)
#endif
	);
#else
	return Tangent(0.f);
#endif
}

Joints GetVertexJoints(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	JointBuffer joint_buffer = PTRCAST(JointBuffer, geometry_buffers.joint);

	return Joints(
	PTRVAL(joint_buffer, vertex_index * 4 + 0)
	,PTRVAL(joint_buffer, vertex_index * 4 + 1)
	,PTRVAL(joint_buffer, vertex_index * 4 + 2)
	,PTRVAL(joint_buffer, vertex_index * 4 + 3)
	);
#else
	return Joints(0);
#endif
}

vec4 GetVertexWeights(uint32_t vertex_index)
{
#ifdef HAS_SKIN
	WeightBuffer weight_buffer = PTRCAST(WeightBuffer, geometry_buffers.weight);

	return vec4(
	PTRVAL(weight_buffer, vertex_index * 4 + 0)
	,PTRVAL(weight_buffer, vertex_index * 4 + 1)
	,PTRVAL(weight_buffer, vertex_index * 4 + 2)
	,PTRVAL(weight_buffer, vertex_index * 4 + 3)
	);
#else
	return vec4(0.f);
#endif
}

Index GetIndex(uint32_t offset)
{
	IndexBuffer index_buffer = PTRCAST(IndexBuffer, geometry_buffers.indices);
	return PTRVAL(index_buffer, offset);
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

#endif
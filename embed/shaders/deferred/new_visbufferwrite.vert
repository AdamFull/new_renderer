#include "shader_inputs"

layout(location = 0) out flat uint drawID;
layout(location = 1) out flat uint instanceID;
layout(location = 2) out flat uint vboID;
layout(location = 3) out vec2 texCoord;
//layout(location = 5) smooth out vec4 currentPos;
//layout(location = 6) smooth out vec4 oldPos;

REGISTER_RO_BUFFER_PTR(PositionBuffer, vec3, 8);
#if defined(COLOR_VEC3)
REGISTER_RO_BUFFER_PTR(ColorBuffer, vec3, 8);
#elif defined(COLOR_VEC4)
REGISTER_RO_BUFFER_PTR(ColorBuffer, vec4, 8);
#endif // COLOR_VEC4
#ifdef HAS_TEXCOORD
REGISTER_RO_BUFFER_PTR(TexcoordBuffer, vec2, 8);
#endif // HAS_TEXCOORD
#ifdef HAS_NORMAL
REGISTER_RO_BUFFER_PTR(NormalBuffer, vec3, 8);
#endif // HAS_NORMAL
#if defined(TANGENT_VEC3)
REGISTER_RO_BUFFER_PTR(TangentBuffer, vec3, 8);
#elif defined(TANGENT_VEC4)
REGISTER_RO_BUFFER_PTR(TangentBuffer, vec4, 8);
#endif // TANGENT_VEC4

#ifdef HAS_SKIN
#if defined(JOINTS_8BIT)
REGISTER_RO_BUFFER_PTR(JointBuffer, uint8_t, 8);
#elif defined(JOINTS_16BIT)
REGISTER_RO_BUFFER_PTR(JointBuffer, uint16_t, 8);
#endif
REGISTER_RO_BUFFER_PTR(WeightBuffer, vec4, 8);
#endif // HAS_SKIN

layout(std140, set = 1, binding = 0) uniform GeometryBuffers
{
	uint64_t position;
	uint64_t color;
	uint64_t texcoord;
	uint64_t normal;
	uint64_t tangent;
	uint64_t joint;
	uint64_t weight;
} geometry_buffers;

layout(push_constant) uniform UniformMeshData
{
	uint32_t callID;
} meshData;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	uint32_t draw_index = meshData.callID + gl_DrawIDARB + gl_InstanceIndex;
	
	drawID = meshData.callID + gl_DrawIDARB;
	instanceID = gl_InstanceIndex;
	//vboID = draw_info.vboID;

	vec3 position = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(PositionBuffer, geometry_buffers.position), gl_VertexIndex);
#ifdef HAS_TEXCOORD
	texCoord = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(TexcoordBuffer, geometry_buffers.texcoord), gl_VertexIndex);
#endif

	vec4 clipSpacePos = vec4(position, 1.f);

	gl_Position = clipSpacePos;
}
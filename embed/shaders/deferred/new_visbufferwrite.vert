#include "shader_inputs"

layout(location = 0) out flat uint drawID;
layout(location = 1) out flat uint instanceID;
layout(location = 2) out flat uint vboID;
layout(location = 3) out vec2 texCoord;
//layout(location = 5) smooth out vec4 currentPos;
//layout(location = 6) smooth out vec4 oldPos;

REGISTER_RO_BUFFER_PTR(PositionBuffer, float, 4);
#if defined(COLOR_VEC3)
REGISTER_RO_BUFFER_PTR2(ColorBuffer, vec3);
#elif defined(COLOR_VEC4)
REGISTER_RO_BUFFER_PTR2(ColorBuffer, vec4);
#endif // COLOR_VEC4
#ifdef HAS_TEXCOORD
REGISTER_RO_BUFFER_PTR2(TexcoordBuffer, vec2);
#endif // HAS_TEXCOORD
#ifdef HAS_NORMAL
REGISTER_RO_BUFFER_PTR2(NormalBuffer, vec3);
#endif // HAS_NORMAL
#if defined(TANGENT_VEC3)
REGISTER_RO_BUFFER_PTR2(TangentBuffer, vec3);
#elif defined(TANGENT_VEC4)
REGISTER_RO_BUFFER_PTR2(TangentBuffer, vec4);
#endif // TANGENT_VEC4

#ifdef HAS_SKIN
#if defined(JOINTS_8BIT)
REGISTER_RO_BUFFER_PTR2(JointBuffer, uint8_t);
#elif defined(JOINTS_16BIT)
REGISTER_RO_BUFFER_PTR2(JointBuffer, uint16_t);
#endif
REGISTER_RO_BUFFER_PTR2(WeightBuffer, vec4);
#endif // HAS_SKIN

#if defined(INDEX_16BIT)
REGISTER_RO_BUFFER_PTR2(IndexBuffer, uint16_t);
#else
REGISTER_RO_BUFFER_PTR2(IndexBuffer, uint32_t);
#endif

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

	FDrawIndexedIndirectData draw_command = getDrawIndexedCommand(drawID);

	// Get vertex offset
	//uint32_t vertex_index = gl_VertexIndex + draw_command.vertexOffset;

	//Index vertex_index = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(IndexBuffer, geometry_buffers.indices), gl_VertexIndex);

	Index vertex_index = Index(gl_VertexIndex);

	float position_x = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(PositionBuffer, geometry_buffers.position), vertex_index * 3 + 0);
	float position_y = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(PositionBuffer, geometry_buffers.position), vertex_index * 3 + 1);
	float position_z = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(PositionBuffer, geometry_buffers.position), vertex_index * 3 + 2);
//#ifdef HAS_TEXCOORD
//	texCoord = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER_RAW(TexcoordBuffer, geometry_buffers.texcoord), vertex_index);
//#endif

	FCamera camera = GetCamera(0);
	mat4 view_projection = getViewProjectionMatrix(camera.matrices_offset);
	
	vec4 clipSpacePos = view_projection * vec4(position_x, position_y, position_z, 1.f);
	//vec4 clipSpacePos = view_projection * vec4(inPosition, 1.f);

	gl_Position = clipSpacePos;
}
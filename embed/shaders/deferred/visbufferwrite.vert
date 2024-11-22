#include "shader_inputs.glsl"

layout(location = 0) out flat uint drawID;
layout(location = 1) out flat uint instanceID;
layout(location = 2) out flat uint vboID;
layout(location = 3) out vec2 texCoord;

layout(push_constant) uniform UniformCallData
{
	uint32_t call_id;
	uint32_t camera_id;
} call_data;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	uint32_t draw_index = call_data.call_id + gl_DrawIDARB + gl_InstanceIndex;
	DrawDescription draw_info = GetDrawInfo(draw_index);

	FSceneNode node = GetSceneNode(draw_info.nodeID);
	mat4 model_matrix = GetModelMatrix(node.matrixOffset);

	uint draw_id = call_data.call_id + gl_DrawIDARB;

	uint32_t vertex_index = uint32_t(gl_VertexIndex);

	// Calculate skinning matrix
#ifdef HAS_SKIN
	Joints joints = GetVertexJoints(vertex_index);
	vec4 weights = GetVertexWeights(vertex_index);

	mat4 skin_mat = 
			weights.x * GetMatrixById(node.skeletonID, joints.x) +
			weights.y * GetMatrixById(node.skeletonID, joints.y) +
			weights.z * GetMatrixById(node.skeletonID, joints.z) +
			weights.w * GetMatrixById(node.skeletonID, joints.w);
	
		model_matrix = model_matrix * skin_mat;
#endif

	Camera camera = GetCamera(call_data.camera_id);
	mat4 view_projection = GetViewProjectionMatrix(camera.matrices_offset);

	// Sample vertex position
	vec3 position = GetVertexPosition(vertex_index);
	vec4 local_pos = model_matrix * (vec4(position, 1.0));
	local_pos = vec4(local_pos.xyz / local_pos.w, 1.0);

	drawID = draw_id;
	instanceID = gl_InstanceIndex;
	//vboID = draw_info.vboID;
	texCoord = GetVertexTexcoord(vertex_index);
	gl_Position = view_projection * local_pos;
}
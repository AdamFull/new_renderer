#include "shader_inputs.glsl"

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outPos;
layout(location = 2) flat out uint material;

layout(push_constant) uniform ShadowmapData
{
	uint32_t call_id;
	int32_t shadow_index;
	int32_t layer;
} shadow_data;

void main()
{
	uint32_t draw_index = shadow_data.call_id + gl_DrawIDARB + gl_InstanceIndex;
	DrawDescription draw_info = GetDrawInfo(draw_index);

	SceneNode node = GetSceneNode(draw_info.node_index);
	mat4 model_matrix = GetModelMatrix(node.matrixOffset);

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

	Mesh mesh = GetMesh(draw_info.mesh_index);

	//model_matrix = model_matrix * MakeDecodePositionMatrix(mesh.bounds_enc);
		
	vec3 position = GetVertexPosition(vertex_index);
	vec4 local_pos = model_matrix * (vec4(position, 1.0));
	outPos = vec4(local_pos.xyz / local_pos.w, 1.0);

#ifdef HAS_TEXCOORD
	outTexcoord = GetVertexTexcoord(vertex_index, mesh.uv_bounds);
#endif

	mat4 viewProj;
#if defined(CASCADED_SHADOW)
	CascadedShadowCommit shadow = GetCascadedShadowCommit(shadow_data.shadow_index);
	viewProj = GetViewProjectionMatrix(shadow.matrix_offsets[shadow_data.layer]);
	gl_Layer = shadow_data.shadow_index * int(cascade_shadow_map_cascade_count) + shadow_data.layer;
#elif defined(OMNI_SHADOW)
	OmniShadowCommit shadow = GetOmniShadowCommit(shadow_data.shadow_index);
	viewProj = GetViewProjectionMatrix(shadow.matrix_offsets[shadow_data.layer]);
	gl_Layer = shadow_data.shadow_index * 6 + shadow_data.layer;
#else
	SpotShadowCommit shadow = GetSpotShadowCommit(shadow_data.shadow_index);
	viewProj = GetViewProjectionMatrix(shadow.matrix_offset);
	gl_Layer = shadow_data.layer;
#endif

	material = ~0u;

	if(mesh.material != ~0u)
	{
		material = mesh.material;
	}

	
	gl_Position = viewProj * outPos;
}
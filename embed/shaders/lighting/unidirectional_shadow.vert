#include "shader_inputs"

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec4 outPos;
layout(location = 2) flat out uint materialID;

layout(push_constant) uniform ShadowmapData
{
	uint32_t callID;
	int32_t shadowIndex;
	int32_t layer;
} shadowData;

void main()
{
	uint32_t draw_index = shadowData.callID + gl_DrawIDARB + gl_InstanceIndex;
	FGeometryBatch draw_info = GetDrawInfo(draw_index);

	FSceneNode node = GetSceneNode(draw_info.nodeID);
	mat4 modelMatrix = GetModelMatrix(node.matrixOffset);

#ifdef HAS_SKIN
	if(node.skeletonID != ~0u)
	{
#if defined(JOINTS_8BIT)
	u8vec4 joints = u8vec4(uint8_t(inJoints.x * 255.0), uint8_t(inJoints.y * 255.0), uint8_t(inJoints.z * 255.0), uint8_t(inJoints.w * 255.0));
#elif defined(JOINTS_16BIT)
	u16vec4 joints = u16vec4(uint16_t(inJoints.x * 65535.0), uint16_t(inJoints.y * 65535.0), uint16_t(inJoints.z * 65535.0), uint16_t(inJoints.w * 65535.0));
#endif
	mat4 skinMat = 
		inWeights.x * GetMatrixById(node.skeletonID, joints.x) +
		inWeights.y * GetMatrixById(node.skeletonID, joints.y) +
		inWeights.z * GetMatrixById(node.skeletonID, joints.z) +
		inWeights.w * GetMatrixById(node.skeletonID, joints.w);
		
		modelMatrix = modelMatrix * skinMat;
	}
#endif // HAS_SKIN
		
	outPos = modelMatrix * vec4(inPosition, 1.0);
	outPos = vec4(outPos.xyz / outPos.w, 1.0);

#ifdef HAS_TEXCOORD
	outTexcoord = inTexCoord;
#endif

	mat4 viewProj;
#if defined(CASCADED_SHADOW)
	FCascadedShadowCommit shadow = GetCascadedShadowCommit(shadowData.shadowIndex);
	FCamera camera = GetCamera(draw_info.cameraID);
	viewProj = GetViewProjectionMatrix(camera.matrices_offset);
	gl_Layer = shadowData.shadowIndex * int(cascade_shadow_map_cascade_count) + shadowData.layer;
#elif defined(OMNI_SHADOW)
	FOmniShadowCommit shadow = GetOmniShadowCommit(shadowData.shadowIndex);
	FCamera camera = GetCamera(draw_info.cameraID);
	viewProj = GetViewProjectionMatrix(camera.matrices_offset);
	gl_Layer = shadowData.shadowIndex * 6 + shadowData.layer;
#else
	FSpotShadowCommit shadow = GetSpotShadowCommit(shadowData.shadowIndex);
	FCamera camera = GetCamera(draw_info.cameraID);
	viewProj = GetViewProjectionMatrix(camera.matrices_offset);
	gl_Layer = shadowData.layer;
#endif

	materialID = ~0u;

	FMesh mesh = GetMesh(draw_info.meshID);
	if(mesh.materialID != ~0u)
	{
		materialID = mesh.materialID;
	}

	
	gl_Position = viewProj * outPos;
}
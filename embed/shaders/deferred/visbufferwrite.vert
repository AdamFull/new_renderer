#version 450

#include "shader_inputs"

layout(location = 0) out flat uint drawID;
layout(location = 1) out flat uint instanceID;
layout(location = 2) out flat uint vboID;
layout(location = 3) out vec2 texCoord;
//layout(location = 5) smooth out vec4 currentPos;
//layout(location = 6) smooth out vec4 oldPos;

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
	FGeometryBatch draw_info = getDrawInfo(draw_index);

	// Calculate skinned matrix from weights and joint indices of the current vertex
	FSceneNode node = getSceneNode(draw_info.nodeID);
	mat4 modelMatrix = getModelMatrix(node.matrixOffset);
	//mat4 oldModelMatrix = getOldModelMatrix(node.matrixOffset)

AS_SKIN
	if(node.skeletonID != ~0u)
	{
ned(JOINTS_8BIT)
		u8vec4 joints = u8vec4(uint8_t(inJoints.x * 255.0), uint8_t(inJoints.y * 255.0), uint8_t(inJoints.z * 255.0), uint8_t(inJoints.w * 255.0));
fined(JOINTS_16BIT)
		u16vec4 joints = u16vec4(uint16_t(inJoints.x * 65535.0), uint16_t(inJoints.y * 65535.0), uint16_t(inJoints.z * 65535.0), uint16_t(inJoints.w * 65535.0));

		mat4 skinMat = 
			inWeights.x * getMatrixById(node.skeletonID, joints.x) +
			inWeights.y * getMatrixById(node.skeletonID, joints.y) +
			inWeights.z * getMatrixById(node.skeletonID, joints.z) +
			inWeights.w * getMatrixById(node.skeletonID, joints.w);

		modelMatrix = modelMatrix * skinMat;
		// oldModelMatrix = oldModelMatrix * skinMat; // TODO: should keep skinMat from prev frame
	}
/ HAS_SKIN

	FCamera camera = GetCamera(draw_info.cameraID);

	vec4 localPos = modelMatrix * (vec4(inPosition, 1.0));
	localPos = vec4(localPos.xyz / localPos.w, 1.0);

	mat4 viewProjection = getViewProjectionMatrix(camera.matricesOffset);

	//currentPos = viewProjection * localPos;

	//vec4 oldLocalPos = oldModelMatrix * vec4(inPosition, 1.0);
	//oldLocalPos = vec4(oldLocalPos.xyz / oldLocalPos.w, 1.0);
	
	//oldPos = viewProjection * oldLocalPos;

	uint subsampleIndex = frameData.frameNumber % 2;
	vec2 texelSize = 0.25f / camera.size;

	vec4 clipSpacePos = viewProjection * localPos;

	//vec4 viewSpace = clipSpacePos / clipSpacePos.w;
	//viewSpace.x += ((subsampleIndex == 0) ? -texelSize.x : texelSize.x);
	//clipSpacePos = viewSpace * clipSpacePos.w;

	drawID = meshData.callID + gl_DrawIDARB;
	instanceID = gl_InstanceIndex;
	vboID = draw_info.vboID;
	texCoord = inTexCoord;

	gl_Position = clipSpacePos;
}
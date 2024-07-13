#ifndef SHADER_INPUTS_GLSL
#define SHADER_INPUTS_GLSL

#include "interop/base"

	// Vertex inputs
#ifdef VERTEX_INPUT
	layout(location = 0) in vec3 inPosition;
#if defined(COLOR_VEC3)
	layout(location = 1) in vec3 inColor;
#elif defined(COLOR_VEC4)
	layout(location = 1) in vec4 inColor;
#endif // HAS_COLOR
#ifdef HAS_TEXCOORD
	layout(location = 2) in vec2 inTexCoord;
#endif // HAS_TEXCOORD
#ifdef HAS_NORMAL
	layout(location = 3) in vec3 inNormal;
#endif // HAS_NORMAL
#if defined(TANGENT_VEC3)
	layout(location = 4) in vec3 inTangent;
#elif defined(TANGENT_VEC4)
	layout(location = 4) in vec4 inTangent;
#endif // HAS_TANGENT
#ifdef HAS_SKIN
	layout(location = 5) in vec4 inJoints;
	layout(location = 6) in vec4 inWeights;
#endif // HAS_SKIN
#endif // VERTEX_INPUT

// Bindless texture
layout(set = 0, binding = 3) uniform sampler2D globalTextures[];

#define GET_BUFFER_POINTER(pointer_type, buffer_desc) pointer_type(buffer_desc.address)
#define GET_VALUE_PTR_BY_ADDRESS(buffer, index) buffer.data[nonuniformEXT(index)]

FSceneNode getSceneNode(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(NodePtr, nodeBuffer), index); }
FMesh getMesh(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(MeshPtr, meshBuffer), index); }
FMaterial GetMaterial(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(MaterialPtr, materialBuffer), index); }
FLightCommit getLight(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(LightPtr, lightsBuffer), index); }

FCascadedShadowCommit getCascadedShadowCommit(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(CascadedShadowPtr, cascadedShadowBuffer), index); }
FOmniShadowCommit getOmniShadowCommit(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(OmniShadowPtr, omniShadowBuffer), index); }
FSpotShadowCommit getSpotShadowCommit(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(SpotShadowPtr, spotShadowBuffer), index); }

FGeometryBatch getDrawInfo(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(DrawInfoPtr, drawInfoBuffer), index); }
FGeometryBatch getRawDrawInfo(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(DrawInfoPtr, rawDrawInfoBuffer), index); }

vec4 getFrustumSide(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(FrustumSidesPtr, frustumSidesBuffer), index); }

FDrawIndexedIndirectData getDrawIndexedCommand(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(IndirectDrawIndexedComandPtr, indirectCommandBuffer), index); }
uint32_t getDrawCount(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(UInt32Ptr, indirectCountBuffer), index); }

uint32_t getTextureRemapIndex(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(UInt32Ptr, textureRemapBuffer), index); }
FCamera GetCamera(uint index) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(CameraBuffer, cameraBuffer), index); }

mat4 getMatrixById(uint global_offset, uint local_offset) { return GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(MatrixPtr, matrixBuffer), global_offset + local_offset); }

mat4 getModelMatrix(uint global_offset) { return getMatrixById(global_offset, model_matrix_offset); }
mat4 getOldModelMatrix(uint global_offset) { return getMatrixById(global_offset, old_model_matrix_offset); }

mat4 getViewMatrix(uint global_offset) { return getMatrixById(global_offset, view_matrix_offset); }
mat4 getInvViewMatrix(uint global_offset) { return getMatrixById(global_offset, inv_view_matrix_offset); }
mat4 getProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, projection_matrix_offset); }
mat4 getInvProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, inv_projection_matrix_offset); }
mat4 getViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, view_projection_matrix_offset); }
mat4 getPrevViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, prev_view_projection_matrix_offset); }
mat4 getInvViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, inv_view_projection_matrix_offset); }
mat4 getReprojectionMatrix(uint global_offset) { return getMatrixById(global_offset, reprojection_matrix_offset); }

#define GET_SAMPLER_2D(texture_index, mip_level) globalTextures[getTextureRemapIndex(texture_index) + mip_level]

vec4 sampleTexture2D(uint32_t texture_index, uint32_t mip_level, vec2 uv)
{
	return texture(GET_SAMPLER_2D(texture_index, mip_level), uv);
}

vec4 sampleTexture2DGrad(uint32_t texture_index, uint32_t mip_level, vec2 uv, vec2 uv_dx, vec2 uv_dy)
{
	return textureGrad(GET_SAMPLER_2D(texture_index, mip_level), uv, uv_dx, uv_dy);
}

//#if defined(COMPUTE_SHADER)
//		layout(std140, set = 1, binding = 0) uniform ComputePassCommonData
//		{
//			vec2 dispatchGridDim; 
//		};
//#elif defined(VERTEX_SHADER) || defined(TESS_CONTROL_SHADER) || defined(TESS_EVAL_SHADER) || defined(GEOMETRY_SHADER) || defined(FRAGMENT_SHADER)
//		layout(std140, set = 1, binding = 0) uniform GraphicsPassCommonData
//		{
//			uint32_t viewportIndex;
//		};
//#endif

#endif // SHADER_INPUTS_GLSL
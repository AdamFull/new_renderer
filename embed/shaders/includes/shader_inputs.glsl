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

#define PTRCAST(pointer_type, address) pointer_type(address)
#define GET_BUFFER_POINTER(pointer_type, buffer_desc) pointer_type(buffer_desc.address)
#define PTRVAL(buffer, index) buffer.data[nonuniformEXT(index)]

FSceneNode GetSceneNode(uint index) { return PTRVAL(GET_BUFFER_POINTER(NodePtr, nodeBuffer), index); }
FMesh GetMesh(uint index) { return PTRVAL(GET_BUFFER_POINTER(MeshPtr, meshBuffer), index); }
FMaterial GetMaterial(uint index) { return PTRVAL(GET_BUFFER_POINTER(MaterialPtr, materialBuffer), index); }
FLightCommit getLight(uint index) { return PTRVAL(GET_BUFFER_POINTER(LightPtr, lightsBuffer), index); }

FCascadedShadowCommit GetCascadedShadowCommit(uint index) { return PTRVAL(GET_BUFFER_POINTER(CascadedShadowPtr, cascadedShadowBuffer), index); }
FOmniShadowCommit GetOmniShadowCommit(uint index) { return PTRVAL(GET_BUFFER_POINTER(OmniShadowPtr, omniShadowBuffer), index); }
FSpotShadowCommit GetSpotShadowCommit(uint index) { return PTRVAL(GET_BUFFER_POINTER(SpotShadowPtr, spotShadowBuffer), index); }

DrawDescription GetDrawInfo(uint index) { return PTRVAL(GET_BUFFER_POINTER(DrawInfoPtr, drawInfoBuffer), index); }
DrawDescription GetRawDrawInfo(uint index) { return PTRVAL(GET_BUFFER_POINTER(DrawInfoPtr, rawDrawInfoBuffer), index); }

vec4 GetFrustumSide(uint index) { return PTRVAL(GET_BUFFER_POINTER(FrustumSidesPtr, frustumSidesBuffer), index); }

FDrawIndexedIndirectData GetDrawIndexedCommand(uint index) { return PTRVAL(GET_BUFFER_POINTER(IndirectDrawIndexedComandPtr, indirectCommandBuffer), index); }
uint32_t GetDrawCount(uint index) { return PTRVAL(GET_BUFFER_POINTER(UInt32Ptr, indirectCountBuffer), index); }

FCamera GetCamera(uint index) { return PTRVAL(GET_BUFFER_POINTER(CameraBuffer, cameraBuffer), index); }

mat4 GetMatrixById(uint global_offset, uint local_offset) { return PTRVAL(GET_BUFFER_POINTER(MatrixPtr, matrixBuffer), global_offset + local_offset); }

mat4 GetModelMatrix(uint global_offset) { return GetMatrixById(global_offset, model_matrix_offset); }
mat4 GetOldModelMatrix(uint global_offset) { return GetMatrixById(global_offset, old_model_matrix_offset); }

mat4 GetViewMatrix(uint global_offset) { return GetMatrixById(global_offset, view_matrix_offset); }
mat4 GetInvViewMatrix(uint global_offset) { return GetMatrixById(global_offset, inv_view_matrix_offset); }
mat4 GetProjectionMatrix(uint global_offset) { return GetMatrixById(global_offset, projection_matrix_offset); }
mat4 GetInvProjectionMatrix(uint global_offset) { return GetMatrixById(global_offset, inv_projection_matrix_offset); }
mat4 GetViewProjectionMatrix(uint global_offset) { return GetMatrixById(global_offset, view_projection_matrix_offset); }
mat4 GetPrevViewProjectionMatrix(uint global_offset) { return GetMatrixById(global_offset, prev_view_projection_matrix_offset); }
mat4 GetInvViewProjectionMatrix(uint global_offset) { return GetMatrixById(global_offset, inv_view_projection_matrix_offset); }
mat4 GetReprojectionMatrix(uint global_offset) { return GetMatrixById(global_offset, reprojection_matrix_offset); }

#define GET_SAMPLER_2D(texture_index, mip_level) globalTextures[texture_index + mip_level]

vec4 SampleTexture2D(uint32_t texture_index, uint32_t mip_level, vec2 uv)
{
	return texture(GET_SAMPLER_2D(texture_index, mip_level), uv);
}

vec4 SampleTexture2DGrad(uint32_t texture_index, uint32_t mip_level, vec2 uv, vec2 uv_dx, vec2 uv_dy)
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
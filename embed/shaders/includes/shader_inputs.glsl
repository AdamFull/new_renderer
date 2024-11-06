#ifndef SHADER_INPUTS_GLSL
#define SHADER_INPUTS_GLSL

#include "geometry"

// Bindless texture
layout(set = 0, binding = 3) uniform sampler2D globalTextures[];

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

#endif // SHADER_INPUTS_GLSL
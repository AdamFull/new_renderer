#ifndef SHADER_INPUTS_GLSL
#define SHADER_INPUTS_GLSL

#include "geometry.glsl"

#extension GL_ARB_shader_draw_parameters : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_ARB_shader_viewport_layer_array : require

// Bindless texture
layout(set = 0, binding = 3) uniform texture2D globalTextures[MAX_SAMPLED_TEXTURES];
layout(set = 0, binding = 4) uniform sampler globalSamplers[MAX_SAMPLED_TEXTURES];

SceneNode GetSceneNode(uint index) { return nodeBuffer[index].data; }
Mesh GetMesh(uint index) { return meshBuffer[index].data; }
Material GetMaterial(uint index) { return materialBuffer[index].data; }
LightCommit getLight(uint index) { return lightsBuffer[index].data; }

CascadedShadowCommit GetCascadedShadowCommit(uint index) { return cascadedShadowBuffer[index].data; }
OmniShadowCommit GetOmniShadowCommit(uint index) { return omniShadowBuffer[index].data; }
SpotShadowCommit GetSpotShadowCommit(uint index) { return spotShadowBuffer[index].data; }

vec4 GetFrustumSide(uint index) { return frustumSidesBuffer[index].data; }

Camera GetCamera(uint index) { return cameraBuffer[index].data; }

mat4 GetMatrixById(uint global_offset, uint local_offset) { return matrixBuffer[global_offset + local_offset].data; }

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

#define GET_SAMPLER_2D(texture_index, mip_level) sampler2D(globalTextures[texture_index + mip_level], globalSamplers[texture_index + mip_level])

vec4 SampleTexture2D(uint32_t texture_index, uint32_t mip_level, vec2 uv)
{
	return texture(GET_SAMPLER_2D(texture_index, mip_level), uv);
}

vec4 SampleTexture2DGrad(uint32_t texture_index, uint32_t mip_level, vec2 uv, vec2 uv_dx, vec2 uv_dy)
{
	return textureGrad(GET_SAMPLER_2D(texture_index, mip_level), uv, uv_dx, uv_dy);
}

#endif // SHADER_INPUTS_GLSL
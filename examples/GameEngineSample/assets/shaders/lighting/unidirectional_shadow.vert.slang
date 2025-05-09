#include "unidirectional_shadow.h"
#include "../include/bindles_layout.slang"

[[vk::push_constant]] ConstantBuffer<shadows::PushConstants> pc;

[[vk::binding(0, 1)]] ConstantBuffer<interop::EncodedGeometryData> egd : register(c0, space1);

struct VSOutput {
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
    nointerpolation uint32_t material : TEXCOORD1;
    float4 position : SV_Position;
    uint32_t layer : SV_RenderTargetArrayIndex;
};

[shader("vertex")]
VSOutput main(uint32_t vertex_index: SV_VertexID, 
              uint32_t instance_index: SV_InstanceID,
              uint32_t draw_index: SV_DrawIndex) {

    uint32_t draw_index = pc.call_id + draw_index + instance_index;
    interop::DrawDescription draw_info = egd.draw_descriptions_ptr[draw_index];

    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

    float4x4 model_matrix = g_fd.get_model_matrix(mesh.matrix_offset);

#ifdef HAS_SKIN
    if (mesh.skin_offset != ~0u) {
        joints_t joints = egd.joints[vertex_index];
        float4 weights = egd.weights[vertex_index];

        float4x4 skin_mat = 
				        weights.x * g_fd.material_ptr[mesh.skin_offset + joints.x] +
                            weights.y * g_fd.material_ptr[mesh.skin_offset + joints.y] +
                            weights.z * g_fd.material_ptr[mesh.skin_offset + joints.z] +
                            weights.w * g_fd.material_ptr[mesh.skin_offset + joints.w];

        model_matrix = mul(model_matrix, skin_mat);
    }
#endif

    VSOutput output;

    float4x4 view_projection;
#if defined(CASCADED_SHADOW)
    interop::CascadedShadowCommit shadow = g_fd.cascaded_shadow_ptr[pc.shadow_index];
    view_projection = g_fd.get_view_projection_matrix(shadow.matrix_offsets[pc.layer]);
    output.layer = pc.shadow_index * int(constants::cascade_shadow_map_cascade_count) + pc.layer;
#elif defined(OMNI_SHADOW)
    interop::OmniShadowCommit shadow = g_fd.omni_shadow_ptr[pc.shadow_index];
    view_projection = g_fd.get_view_projection_matrix(shadow.matrix_offsets[pc.layer]);
    output.layer = pc.shadow_index * 6 + pc.layer;
#else
    interop::SpotShadowCommit shadow = g_fd.spot_shadow_ptr[pc.shadow_index];
    view_projection = g_fd.get_view_projection_matrix(shadow.matrix_offset);
    output.layer = pc.layer;
#endif

    output.material = submesh.material;
    output.pos = mul(model_matrix, float4(interop::decode_position(egd.position[vertex_index]), 1.0));
    output.pos = float4(output.pos.xyz / output.pos.w, 1.0);
    output.texcoord = interop::decode_texcoord(egd.texcoord[vertex_index], mesh.uv_enc);
    output.position = mul(view_projection, output.pos);

    return output;
}
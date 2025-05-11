#include "visbufferwrite.h"
#include "../include/bindles_layout.slang"

[[vk::push_constant]] ConstantBuffer<visbufferwrite::PushConstants> pc;

[[vk::binding(0, 1)]] ConstantBuffer<interop::EncodedGeometryData> egd : register(c0, space1);

struct VSOutput {
    nointerpolation uint32_t draw_id : TEXCOORD0;
    nointerpolation uint32_t instance_id : TEXCOORD1;
    nointerpolation uint32_t vbo_id : TEXCOORD2;
    float2 texcoord : TEXCOORD3;
    float4 position : SV_Position;
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

    uint32_t draw_id = pc.call_id + draw_index;
    
    // Calculate skinning matrix
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

    interop::Camera camera = g_fd.camera_ptr[pc.camera_id];
    float4x4 view_projection = g_fd.get_view_projection_matrix(camera.matrices_offset);

    // Sample vertex position
    float4 local_pos = mul(model_matrix, float4(interop::decode_position(egd.vertices[vertex_index].position), 1.0));

    VSOutput output;
    output.draw_id = draw_id;
    output.instance_id = instance_index;
    output.vbo_id = 0;
    output.texcoord = interop::decode_texcoord(egd.vertices[vertex_index].uv, mesh.uv_enc);
    output.position = mul(view_projection, local_pos);

    return output;
}
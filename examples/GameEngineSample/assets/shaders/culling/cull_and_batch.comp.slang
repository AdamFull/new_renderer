#include "cull_and_batch.h"
#include "../include/bindles_layout.slang"

[[vk::push_constant]] ConstantBuffer<cull_and_batch::PushConstants> pc;

inline bool check_mesh_in_frustum(in interop::BoundingBox bb, in uint32_t frustum_offset) {
    float4 *camera_frustum = g_fd.frustum_sides_ptr + frustum_offset; 

    [unroll]
    for (uint32_t i = 0; i < 6; i++) {
        float4 edge = camera_frustum[i];
        // TODO: add this check to BoundingBox
        if (edge.x * bb.min.x + edge.y * bb.min.y + edge.z * bb.min.z + edge.w <= 0.0f
			&& edge.x * bb.max.x + edge.y * bb.min.y + edge.z * bb.min.z + edge.w <= 0.0f
			&& edge.x * bb.min.x + edge.y * bb.max.y + edge.z * bb.min.z + edge.w <= 0.0f
			&& edge.x * bb.max.x + edge.y * bb.max.y + edge.z * bb.min.z + edge.w <= 0.0f
			&& edge.x * bb.min.x + edge.y * bb.min.y + edge.z * bb.max.z + edge.w <= 0.0f
			&& edge.x * bb.max.x + edge.y * bb.min.y + edge.z * bb.max.z + edge.w <= 0.0f
			&& edge.x * bb.min.x + edge.y * bb.max.y + edge.z * bb.max.z + edge.w <= 0.0f
			&& edge.x * bb.max.x + edge.y * bb.max.y + edge.z * bb.max.z + edge.w <= 0.0f)
            return false;
    }
    return true;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint32_t3 global_id: SV_DispatchThreadID) {

    if (global_id.x > g_fd.draw_count) {
        return;
	}

    uint32_t global_draw_index = global_id.x;
    interop::DrawDescription draw_info = pc.input_draw_desc_ptr[global_draw_index];

    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

    float4x4 model = g_fd.get_model_matrix(mesh.matrix_offset);

    // Calculate bounding box
    interop::BoundingBox bb = mesh.ebb.decode(model);
    interop::BoundingBox submesh_bb = submesh.ebb.decode(model);

    interop::Camera camera = g_fd.camera_ptr[draw_info.camera_index];
    if (check_mesh_in_frustum(submesh_bb, camera.frustum_offset)) {
        interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
        interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

        interop::DrawIndexedIndirectData indirect_command;
        indirect_command.index_count = submesh.index_count;
        indirect_command.instance_count = 1;
        indirect_command.first_index = submesh.first_index;
        indirect_command.vertex_offset = submesh.first_vertex;
        indirect_command.first_instance = 0;

        pc.output_draw_indirect_ptr[global_draw_index] = indirect_command;
    }
    else {
        pc.output_draw_indirect_ptr[global_draw_index].instance_count = 0;;
    }
}
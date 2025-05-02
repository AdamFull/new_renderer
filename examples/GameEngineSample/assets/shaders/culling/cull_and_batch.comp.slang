#include "cull_and_batch.h"
#include "../include/bindles_layout.slang"

[[vk::push_constant]] ConstantBuffer<cull_and_batch::PushConstants> pc;

inline bool check_mesh_in_frustum(in interop::BoundingBox bb, in uint32_t frustum_offset) {
    [unroll]
    for (uint32_t i = 0; i < 6; i++) {
        float4 edge = g_fd.frustum_sides_ptr[frustum_offset + i];
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

    // Compute the index of the bit in the bit array
    uint32_t word_index = global_draw_index >> 5; // Divide by 32
    uint32_t bit_index = global_draw_index & 31;  // Modulo 32
    uint32_t bit_mask = 1u << bit_index;

    interop::DrawDescription draw_info = pc.draw_desc_ptr[global_draw_index];

    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

    float4x4 model = g_fd.get_model_matrix(mesh.matrix_offset);

    // Calculate bounding box
    interop::BoundingBox bb = mesh.ebb.decode(model);
    interop::BoundingBox submesh_bb = submesh.ebb.decode(model);

    interop::Camera camera = g_fd.camera_ptr[draw_info.camera_index];
    if (check_mesh_in_frustum(submesh_bb, camera.frustum_offset)) {
        // Instance culled
        InterlockedOr(pc.alive_meshes_bit_array_ptr[word_index], bit_mask);
    }
    else {
        // Instance skipped
        InterlockedAnd(pc.alive_meshes_bit_array_ptr[word_index], ~bit_mask);
    }

    // atomicAdd(pc.draw_counts[draw_info.camera_index].data, 1);
    //
    // SubMesh submesh = GetSubMesh(mesh.submesh_offset + draw_info.submesh_index);
    //
    // DrawIndexedIndirectData indirect_command;
    // indirect_command.indexCount = submesh.index_count;
    // indirect_command.instanceCount = 1;
    // indirect_command.firstIndex = submesh.first_index;
    // indirect_command.vertexOffset = submesh.first_vertex;
    // indirect_command.firstInstance = 0;
    //
    // pc.alive_draw_commands[global_draw_index].data = indirect_command;
    // pc.alive_descriptions[global_draw_index].data = draw_info;
}
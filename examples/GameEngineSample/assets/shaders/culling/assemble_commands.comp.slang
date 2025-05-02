#include "../include/bindles_layout.slang"
#include "../include/compute_common.slang"
#include "assemble_commands.h"

[[vk::push_constant]] ConstantBuffer<assemble_commands::PushConstants> pc;

uint32_t sample_bit(uint32_t global_id) {
    // Handle bit array input
    uint32_t word_index = global_id >> 5; // Divide by 32
    uint32_t bit_index = global_id & 31;  // Modulo 32
    uint32_t word = pc.alive_bits[word_index];
    return (word >> bit_index) & 1u;
}

#define GROUP_SIZE 256

[numthreads(GROUP_SIZE, 1, 1)]
[shader("compute")]
void main(uint32_t3 global_id: SV_DispatchThreadID) {

    uint32_t global_index = global_id.x;
    if (global_index >= g_fd.draw_count) {
        return;
    }

    // Extract visibility bit (0 = culled, 1 = visible)
    uint32_t bit = sample_bit(global_index);

    interop::DrawDescription draw_info = pc.source_descriptions[global_index];

    if (bit == 1) {
        interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
        interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

        interop::DrawIndexedIndirectData indirect_command;
        indirect_command.index_count = submesh.index_count;
        indirect_command.instance_count = 1;
        indirect_command.first_index = submesh.first_index;
        indirect_command.vertex_offset = submesh.first_vertex;
        indirect_command.first_instance = 0;

        uint32_t inplace_offset = pc.alive_offsets[global_index];
        // inplece_offset should be converted in to camera space index

        pc.alive_draw_commands[inplace_offset] = indirect_command;
        pc.alive_descriptions[inplace_offset] = draw_info;

        InterlockedAdd(pc.draw_counts[draw_info.camera_index], 1);
    }
}

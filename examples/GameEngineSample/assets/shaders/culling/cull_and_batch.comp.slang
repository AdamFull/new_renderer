#include "cull_and_batch.h"
#include "../include/bindles_layout.slang"
#include "../include/util.slang"

[[vk::binding(0, 1)]] Texture2D<float> texture0 : register(to, space1); // depth pyramid (hiz)
[[vk::binding(1, 1)]] SamplerState sampler0 : register(s0, space1); // depth sampler (nearest)

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

inline float calculate_screen_size(in float3 center, in float radius, in float4x4 view_proj_matrix, in float2 screen_size) {
    // Transform the center of the bounding sphere to clip space
    float4 clip_pos = mul(view_proj_matrix, float4(center, 1.0f));

    // Calculate projected radius
    float w = clip_pos.w;

    // Avoid division by zero
    if (w <= 0.0f) {
        return 0.0f;
    }

    // Project radius to screen space
    float projected_radius = radius / w;

    // Calculate screen size as a fraction of the screen
    float screen_radius = projected_radius * min(screen_size.x, screen_size.y) * 0.5f;

    return screen_radius;
}

inline uint32_t select_mesh_lod(in interop::Camera camera, in interop::BoundingBox bb, in interop::SubMesh submesh) {
    float3 center = (bb.min + bb.max) * 0.5;
    float radius = length(bb.max - center);

    float distance = length(camera.position - center);

    // Define desired screen-space error (in pixels)
    float tau = 1.0f;

    // Compute LOD selection threshold
    float threshold = (tau * distance * 2.0f * tan(camera.fov / 2.0f)) / camera.size.y;

    // Select the appropriate LOD
    uint selected_lod = 0;
    for (uint i = 1; i < submesh.lod_count; i++) {
        interop::MeshLod lod = submesh.lods[i];
        if (lod.error > threshold) {
            break;
        }
        selected_lod = i;
    }

    return selected_lod;
}

bool is_occluded(in interop::Camera camera, in interop::BoundingBox bb) {
    uint32_t2 pyramid_size;
    texture0.GetDimensions(pyramid_size.x, pyramid_size.y);

    float4x4 view_proj_matrix = g_fd.get_view_projection_matrix(camera.matrices_offset);

    float3 corners[8] = {
        bb.min,
        float3(bb.max.x, bb.min.y, bb.min.z),
        float3(bb.min.x, bb.max.y, bb.min.z),
        float3(bb.max.x, bb.max.y, bb.min.z),
        float3(bb.min.x, bb.min.y, bb.max.z),
        float3(bb.max.x, bb.min.y, bb.max.z),
        float3(bb.min.x, bb.max.y, bb.max.z),
        bb.max
    };

    // Compute corners in ndc
    [unroll]
    for (int i = 0; i < 8; ++i) {
        float4 clip_pos = mul(view_proj_matrix, float4(corners[i], 1.0));
        corners[i] = clip_pos.xyz / clip_pos.w;
    }

    float3 ndc_min = corners[0];
    float3 ndc_max = corners[0];

    [unroll]
    for (int i = 1; i < 8; ++i) {
        ndc_min = min(ndc_min, corners[i]);
        ndc_max = max(ndc_max, corners[i]);
    }

    float2 uv_min = util::clip_to_uv(ndc_min.xy);
    float2 uv_max = util::clip_to_uv(ndc_max.xy);

    float2 aabb_size_pixels = (uv_max - uv_min) * float2(pyramid_size);
    float mip_level = ceil(log2(max(max(aabb_size_pixels.x, aabb_size_pixels.y), 1.0)));

    float depth = 1.0f;
    depth = min(depth, texture0.SampleLevel(sampler0, float2(uv_min.x, uv_min.y), mip_level));
    depth = min(depth, texture0.SampleLevel(sampler0, float2(uv_min.x, uv_max.y), mip_level));
    depth = min(depth, texture0.SampleLevel(sampler0, float2(uv_max.x, uv_min.y), mip_level));
    depth = min(depth, texture0.SampleLevel(sampler0, float2(uv_max.x, uv_max.y), mip_level));

    return depth > ndc_max.z;
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
    if (!check_mesh_in_frustum(submesh_bb, camera.frustum_offset)) {
        pc.output_draw_indirect_ptr[global_draw_index].instance_count = 0;
        return;
    }

    //if (is_occluded(camera, submesh_bb) && g_fd.frame_number > 1) {
    //    pc.output_draw_indirect_ptr[global_draw_index].instance_count = 0;
    //    return;
    //}

    uint32_t lod_index = select_mesh_lod(camera, submesh_bb, submesh);
    interop::MeshLod selected_lod = submesh.lods[lod_index];

    interop::DrawIndexedIndirectData indirect_command;
    indirect_command.index_count = selected_lod.index_count;
    indirect_command.instance_count = 1;
    indirect_command.first_index = selected_lod.first_index;
    indirect_command.vertex_offset = submesh.first_vertex;
    indirect_command.first_instance = 0;

    pc.output_draw_indirect_ptr[global_draw_index] = indirect_command;
}
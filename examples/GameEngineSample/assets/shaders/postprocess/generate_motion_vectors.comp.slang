#include "../include/bindles_layout.slang"
#include "../include/compute_common.slang"
#include "../include/util.slang"

[[vk::binding(0, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(1, 1)]] SamplerState sampler0 : register(s0, space1);
[[vk::binding(2, 1)]] WTexture2D<float2> image0 : register(u0, space1);

[numthreads(8, 8, 1)]
[shader("compute")]
void main(uint32_t3 local_id: SV_GroupThreadID,
    uint32_t3 global_id: SV_DispatchThreadID,
          uint32_t3 group_id: SV_GroupID) {
    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    float2 texel_size = 1.0f / float2(image_size);

    uint32_t2 px;
    float2 uv;
    if (swizzle_workgroups_8x8(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    interop::Camera camera = g_fd.get_default_camera();
    float4x4 inv_view_projection = g_fd.get_inv_view_projection_matrix(camera.matrices_offset);
    float4x4 prev_view_projection = g_fd.get_prev_view_projection_matrix(camera.matrices_offset);

    float depth = texture0.SampleLevel(sampler0, uv, 0.0f).r;

    float4 clip_space_location = float4(util::uv_to_clip(uv), depth, 1.0);
    float4 homogenous_location = mul(inv_view_projection, clip_space_location);
    float4 world_pos = homogenous_location / homogenous_location.w;

    float4 prev_clip_space_location = mul(prev_view_projection, world_pos);
    prev_clip_space_location /= prev_clip_space_location.w;

    // Use this frame's position and last frame's to compute the pixel velocity.
    float2 velocity = (clip_space_location.xy - prev_clip_space_location.xy) / 2.f;

    // Encode velocity
    velocity = util::encode_velocity(velocity);

    image0.Store(px, velocity);
}
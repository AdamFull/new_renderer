#include "../include/compute_common.slang"
#include "../include/bindles_layout.slang"
#include "../include/util.slang"
#include "../include/random.slang"

[[vk::binding(0, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(1, 1)]] SamplerState sampler0 : register(s0, space1);
[[vk::binding(2, 1)]] WTexture2D<float> image0 : register(u0, space1);

static const int AO_SAMPLE_COUNT = 16;
static const float AO_RADIUS = 0.2f;
static const float AO_DEPTH_THRESHOLD = 0.0025f;
static const float AO_POWER = 1.2f;

#define TILE_SIZE 8

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
[shader("compute")]
void main(uint32_t3 local_id: SV_GroupThreadID,
    uint32_t3 global_id: SV_DispatchThreadID,
    uint32_t3 group_id: SV_GroupID) {

    uint32_t2 write_px;
    float2 write_uv;

    uint32_t2 write_image_size;
    image0.GetDimensions(write_image_size.x, write_image_size.y);
    float2 write_texel_size = 1.0f / float2(write_image_size);

    uint32_t2 sample_image_size;
    texture0.GetDimensions(sample_image_size.x, sample_image_size.y);
    float2 sample_texel_size = 1.0f / float2(sample_image_size);

    interop::Camera camera = g_fd.get_default_camera();
    float4x4 inv_proj = g_fd.get_inv_projection_matrix(camera.matrices_offset);

    uint32_t2 local_id_xy = local_id.xy;
    uint32_t2 global_id_xy = global_id.xy;

    if (swizzle_workgroups_8x8(global_id_xy, local_id_xy, group_id.xy, write_image_size, write_uv, write_px))
        return;

    int32_t2 sample_px = int32_t2(
        write_px.x * (sample_image_size.x / write_image_size.x),
                          write_px.y * (sample_image_size.y / write_image_size.y)
    );
    float2 sample_uv = float2(sample_px) / float2(sample_image_size);
    
    float c0 = texture0.Load(int32_t3(sample_px + int32_t2(0, 0), 0)).r;
    float l2 = texture0.Load(int32_t3(sample_px + int32_t2(-2, 0), 0)).r;
    float l1 = texture0.Load(int32_t3(sample_px + int32_t2(-1, 0), 0)).r;
    float r1 = texture0.Load(int32_t3(sample_px + int32_t2(1, 0), 0)).r;
    float r2 = texture0.Load(int32_t3(sample_px + int32_t2(2, 0), 0)).r;
    float b2 = texture0.Load(int32_t3(sample_px + int32_t2(0, -2), 0)).r;
    float b1 = texture0.Load(int32_t3(sample_px + int32_t2(0, -1), 0)).r;
    float t1 = texture0.Load(int32_t3(sample_px + int32_t2(0, 1), 0)).r;
    float t2 = texture0.Load(int32_t3(sample_px + int32_t2(0, 2), 0)).r;

    // Depth slope estimates
    float dl = abs(l1 * l2 / (2.0f * l2 - l1) - c0);
    float dr = abs(r1 * r2 / (2.0f * r2 - r1) - c0);
    float db = abs(b1 * b2 / (2.0f * b2 - b1) - c0);
    float dt = abs(t1 * t2 / (2.0f * t2 - t1) - c0);

    // Reconstruct center position
    float3 ce = util::position_from_depth(sample_uv, c0, inv_proj);

    // Calculate derivatives
    float3 dpdx = (dl < dr) ?
        ce - util::position_from_depth(sample_uv - float2(sample_texel_size.x, 0.0f), l1, inv_proj) :
        -ce + util::position_from_depth(sample_uv + float2(sample_texel_size.x, 0.0f), r1, inv_proj);

    float3 dpdy = (db < dt) ?
        ce - util::position_from_depth(sample_uv - float2(0.0f, sample_texel_size.y), b1, inv_proj) :
        -ce + util::position_from_depth(sample_uv + float2(0.0f, sample_texel_size.y), t1, inv_proj);

    float3 normal = normalize(cross(dpdx, dpdy));

    let rng = random::RandomNumberGenerator(global_id_xy, write_image_size, g_fd.frame_number); // frameData.frameNumber

    const float3 random_vec = rng.sample_3d() * 2.0f - 1.0f;

    const float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    const float3 bitangent = cross(normal, tangent);
    const float3x3 tbn = float3x3(tangent, bitangent, normal);

    const float4x4 projection = g_fd.get_projection_matrix(camera.matrices_offset);
    
    float occlusion = 0;
    
    [unroll]
    for (int32_t i = 0; i < AO_SAMPLE_COUNT; ++i) {
        const float2 hamm = random::hammersley2d(i, AO_SAMPLE_COUNT);

        const float ao_range = AO_RADIUS * lerp(0.2, 1.0, rng.sample1d());
        const float3 hemisphere = random::hemispherepoint_uniform(hamm.x, hamm.y);
        const float3 cone = mul(hemisphere, tbn);
        const float3 vpos = ce + cone * ao_range;
    
        float4 new_tex_coord = mul(projection, float4(vpos, 1.0f));
        new_tex_coord.xyz /= new_tex_coord.w;
        new_tex_coord.xy = util::clip_to_uv(new_tex_coord.xy);

        [branch]
        if (all(new_tex_coord.xy >= 0) && all(new_tex_coord.xy <= 1)) {
            float sample_depth = texture0.SampleLevel(sampler0, new_tex_coord.xy, 0.0f).r;
            float linear_depth = -util::linearize_depth(sample_depth, camera.z_near, camera.z_far);
            float range_check = smoothstep(0.0f, 1.0f, AO_RADIUS / abs(ce.z - linear_depth));
            occlusion += (linear_depth >= vpos.z + AO_DEPTH_THRESHOLD) ? range_check : 0.0f;
        }
    }
    
    occlusion /= AO_SAMPLE_COUNT;
    occlusion = pow(saturate(1.0f - occlusion), AO_POWER);

    image0.Store(write_px, occlusion);
}
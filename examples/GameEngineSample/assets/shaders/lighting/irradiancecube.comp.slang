#include "../include/lighting.slang"
#include "../include/compute_common.slang"

WTexture2DArray<float4> image0;
TextureCube<float4> texture0;
SamplerState sampler0;

#define NUM_THREADS_X 8
#define NUM_THREADS_Y 8
#define NUM_THREADS_Z 6
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void main(uint3 global_id: SV_DispatchThreadID, uint3 local_id: SV_GroupThreadID, uint3 group_id: SV_GroupID) {
    constexpr float delta_phi = constants::two_pi / 180.0f;
    constexpr float delta_theta = constants::half_pi / 64.0f;

    uint3 image_size;
    image0.GetDimensions(image_size.x, image_size.y, image_size.z);

    uint3 px = global_id;

    uint2 px_swizzled = uint2(0, 0);
    float2 uv = float2(0, 0);
    if (swizzle_workgroups_8x8(global_id.xy, local_id.xy, group_id.xy, image_size.xy, uv, px_swizzled))
        return;

    px.xy = px_swizzled;

    float3 N = normalize(get_cube_side_direction(uv, px.z));
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);
    
    float3 colour = float3(0.0f);
    uint32_t env_sample_count = 0u;
    
    for (float phi = 0.0f; phi < constants::two_pi; phi += delta_phi) {
        for (float theta = 0.0f; theta < constants::half_pi; theta += delta_theta) {
            float3 temp_vec = cos(phi) * right + sin(phi) * up;
            float3 sample_vector = cos(theta) * N + sin(theta) * temp_vec;
            colour += texture0.SampleLevel(sampler0, sample_vector, 0.f).rgb * cos(theta) * sin(theta);
            env_sample_count++;
        }
    }

    image0.Store(px, float4(constants::pi * colour / float(env_sample_count), 1.0f));
}
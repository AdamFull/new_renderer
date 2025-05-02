#include "../include/bindles_layout.slang"
#include "../include/util.slang"

[[vk::binding(0, 1)]] Texture2D texture0 : register(t0, space1); // ao
[[vk::binding(1, 1)]] SamplerState sampler0 : register(s0, space1); // ao sampler
[[vk::binding(2, 1)]] Texture2D texture1 : register(t1, space1); // depth
[[vk::binding(3, 1)]] SamplerState sampler1 : register(s1, space1); // depth sampler
[[vk::binding(4, 1)]] WTexture2D<float> image0 : register(u0, space1);

#define THREAD_GROUP_SIZE (512)
#define KERNEL_LENGTH (4)
#define BLUR_WEIGHTS (2 * KERNEL_LENGTH + 1)
#define GS_CACHE_SIZE (KERNEL_LENGTH + THREAD_GROUP_SIZE + KERNEL_LENGTH)

groupshared float gsm_ao_cache[GS_CACHE_SIZE];
groupshared float gsm_depth_cache[GS_CACHE_SIZE];

static const float blur_weights[KERNEL_LENGTH + 1] = float[](1.0, 0.9, 0.6, 0.15, 0.1);
static const float normalization_factor = 1.0 /
    (blur_weights[0] + 2.0 * (blur_weights[1] + blur_weights[2] + blur_weights[3] + blur_weights[4]));

static const float blur_weights_normalized[BLUR_WEIGHTS] = float[](
    blur_weights[4] * normalization_factor,
    blur_weights[3] * normalization_factor,
    blur_weights[2] * normalization_factor,
    blur_weights[1] * normalization_factor,
    blur_weights[0] * normalization_factor,
    blur_weights[1] * normalization_factor,
    blur_weights[2] * normalization_factor,
    blur_weights[3] * normalization_factor,
    blur_weights[4] * normalization_factor
);

#ifdef HORIZONTAL_PASS
static const float2 direction = float2(1, 0);
#else
static const float2 direction = float2(0, 1);
#endif

[shader("compute")]
#ifdef HORIZONTAL_PASS
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
#else
[numthreads(1, THREAD_GROUP_SIZE, 1)]
#endif
void main(uint32_t3 group_id: SV_GroupID,
          uint32_t group_idx: SV_GroupIndex) {

    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    const float2 texel_size = 1.0f / float2(image_size);

    const interop::Camera camera = g_fd.get_default_camera();

    const uint32_t2 multiplier = uint32_t2(direction) * uint32_t(THREAD_GROUP_SIZE) + (uint32_t2(1) - uint32_t2(direction));
    const uint32_t2 group_begin = group_id.xy * multiplier;

    for (uint32_t i = group_idx; i < GS_CACHE_SIZE; i += THREAD_GROUP_SIZE) {
        float2 uv = (float2(group_begin) + 0.5 + direction * float(int(i) - KERNEL_LENGTH)) * texel_size;
        gsm_ao_cache[i] = texture0.SampleLevel(sampler0, uv, 0.0).r;
        const float depth = texture1.SampleLevel(sampler1, uv, 0.0).r;
        gsm_depth_cache[i] = util::linearize_depth(depth, camera.z_near, camera.z_far);
    }

    GroupMemoryBarrierWithGroupSync();

    const uint32_t center = group_idx + KERNEL_LENGTH;
    const float current_ao = gsm_ao_cache[center];
    const float current_scene_depth = gsm_depth_cache[center];

    float result = 0;
    for (int i = 0; i < BLUR_WEIGHTS; ++i) {
        const uint32_t idx = center + uint32_t(i) - KERNEL_LENGTH;
        const float d = gsm_depth_cache[idx];
        const float w = clamp(abs(d - current_scene_depth), 0.0, 1.0);
        result += lerp(gsm_ao_cache[idx], current_ao, w) * blur_weights_normalized[i];
    }

    int32_t2 pixel = int32_t2(group_begin) + int32_t2(direction) * int32_t(group_idx);
    image0.Store(pixel, result);
}
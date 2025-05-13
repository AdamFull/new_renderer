#pragma once

#ifdef __cplusplus
#include <cstdint>
#endif

#define FRAME_DATA_SLOT 0
#define SAMPLER_SLOT 1
#define SRV_TEXTURE_SLOT 2
#define UAV_TEXTURE_SLOT 3

#define MAX_SAMPLER_SLOTS 2048
#define MAX_SRV_TEXTURE_SLOTS 1024
#define MAX_UAV_TEXTURE_SLOTS 1024

namespace constants {
	static const uint16_t invalid_index = 0xffff;

	// Math constants and other
	static const float pi = 3.14159265358979323846;
	static const float half_pi = 1.57079632679489661923;
	static const float two_pi = 6.283185307179586476925286766559;
	static const float four_pi = 2.0 * two_pi;
	static const float flt_max = 3.402823466e+38F;
	static const float flt_epsilon = 1.192092896e-07F;
	static const float flt_inf = ((float)(1e+300 * 1e+300));
	static const uint32_t uint_max = 4294967295u;

	// Directional light
	static const uint32_t max_directional_light_count = 3u;
	static const uint32_t max_directional_light_shadow_count = 2u;
	static const uint32_t cascade_shadow_map_resolution = 2048u;
	static const uint32_t cascade_shadow_map_cascade_count = 4u;

	// Spot light
	static const uint32_t max_spot_light_count = 64u;
	static const uint32_t max_spot_light_shadow_count = 2u;
	static const uint32_t spot_light_shadow_map_resolution = 1024u;

	// Point light
	static const uint32_t max_point_light_count = 64u;
	static const uint32_t max_point_light_shadow_count = 2u;
	static const uint32_t point_light_shadow_map_resolution = 512u;

	static const uint32_t model_matrix_offset = 0u;
	static const uint32_t old_model_matrix_offset = 1u;

	static const uint32_t view_matrix_offset = 0u;
	static const uint32_t inv_view_matrix_offset = 1u;
	static const uint32_t projection_matrix_offset = 2u;
	static const uint32_t inv_projection_matrix_offset = 3u;
	static const uint32_t view_projection_matrix_offset = 4u;
	static const uint32_t prev_view_projection_matrix_offset = 5u;
	static const uint32_t inv_view_projection_matrix_offset = 6u;
	static const uint32_t reprojection_matrix_offset = 7u;
	static const uint32_t matrix_offset_count = 8u;

	static const uint32_t max_draw_instance_count = 100000u;

	static const uint32_t max_player_cameras = 2u;
	static const uint32_t max_directional_light_cameras = max_directional_light_shadow_count * cascade_shadow_map_cascade_count;
	static const uint32_t max_spot_light_cameras = max_spot_light_shadow_count;
	static const uint32_t max_omni_light_cameras = max_point_light_shadow_count * 6u;
	static const uint32_t max_cameras = max_player_cameras + max_directional_light_cameras + max_spot_light_cameras + max_omni_light_cameras;

	static const uint32_t max_draw_command_count = max_draw_instance_count * max_cameras;

	static const uint32_t k_max_lod_count = 8;
}
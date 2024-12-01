#ifndef SHLIB_CONSTANTS_H
#define SHLIB_CONSTANTS_H

#include "base.h"

SHLIB_NS_BEGIN

CONSTANT uint32_t MAX_SAMPLED_TEXTURES = 2048u;

CONSTANT uint32_t kShaderGroupSize = 32u;
CONSTANT uint32_t kMaxVerticesPerMeshlet = 64u;
CONSTANT uint32_t kMaxTrianglesPerMeshlet = 124u;
CONSTANT uint32_t kMaxMeshLodCount = 4u;

// Directional light
CONSTANT uint32_t max_directional_light_count = 3u;
CONSTANT uint32_t max_directional_light_shadow_count = 2u;
CONSTANT uint32_t cascade_shadow_map_resolution = 2048u;
CONSTANT uint32_t cascade_shadow_map_cascade_count = 4u;

// Spot light
CONSTANT uint32_t max_spot_light_count = 64u;
CONSTANT uint32_t max_spot_light_shadow_count = 2u;
CONSTANT uint32_t spot_light_shadow_map_resolution = 1024u;

// Point light
CONSTANT uint32_t max_point_light_count = 64u;
CONSTANT uint32_t max_point_light_shadow_count = 2u;
CONSTANT uint32_t point_light_shadow_map_resolution = 512u;

CONSTANT uint32_t model_matrix_offset = 0u;
CONSTANT uint32_t old_model_matrix_offset = 1u;

CONSTANT uint32_t view_matrix_offset = 0u;
CONSTANT uint32_t inv_view_matrix_offset = 1u;
CONSTANT uint32_t projection_matrix_offset = 2u;
CONSTANT uint32_t inv_projection_matrix_offset = 3u;
CONSTANT uint32_t view_projection_matrix_offset = 4u;
CONSTANT uint32_t prev_view_projection_matrix_offset = 5u;
CONSTANT uint32_t inv_view_projection_matrix_offset = 6u;
CONSTANT uint32_t reprojection_matrix_offset = 7u;
CONSTANT uint32_t matrix_offset_count = 8u;


CONSTANT uint32_t light_type_directional = 1u;
CONSTANT uint32_t light_type_point = 2u;
CONSTANT uint32_t light_type_spot = 3u;
CONSTANT uint32_t light_type_invalid = 0u;

CONSTANT uint32_t MAX_DRAW_INSTANCE_COUNT = 100000u;

// Camera constants
CONSTANT uint32_t MAX_PLAYER_CAMERAS = 2u;
CONSTANT uint32_t MAX_DIRECTIONAL_LIGHT_CAMERAS = max_directional_light_shadow_count * cascade_shadow_map_cascade_count;
CONSTANT uint32_t MAX_SPOT_LIGHT_CAMERAS = max_spot_light_shadow_count;
CONSTANT uint32_t MAX_OMNI_LIGHT_CAMERAS = max_point_light_shadow_count * 6u;
CONSTANT uint32_t MAX_CAMERAS = MAX_PLAYER_CAMERAS + MAX_DIRECTIONAL_LIGHT_CAMERAS + MAX_SPOT_LIGHT_CAMERAS + MAX_OMNI_LIGHT_CAMERAS;

// Draw constants
CONSTANT uint32_t MAX_DRAW_COMMAND_COUNT = MAX_DRAW_INSTANCE_COUNT * MAX_CAMERAS;

SHLIB_NS_END

#endif // SHLIB_CONSTANTS_H
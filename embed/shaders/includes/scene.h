#ifndef SHLIB_SCENE_H
#define SHLIB_SCENE_H

#include "mesh.h"

SHLIB_NS_BEGIN

struct SceneNode
{
	uint32_t matrixOffset;
	uint32_t skeletonID;
};

REGISTER_TYPE_PTR(SceneNode, 4);

struct Camera
{
	uint32_t type;
	uint32_t matrices_offset;
	uint32_t frustum_offset;
	uint32_t last_hash;

	float fov;
	float z_near;
	float z_far;
	float aspect;

	vec2 size;
	vec2 invSize;
};

REGISTER_TYPE_PTR(Camera, 4);

struct LightCommit
{
	ALIGN_AS(16) vec3 position;
	ALIGN_AS(16) vec3 direction;
	ALIGN_AS(16) vec3 color;
	float intencity;
	float lightAngleScale;
	float lightAngleOffset;
	int toTarget;
	float radius;
	int shadowIndex;
	int castShadows;
	int type;
};

REGISTER_TYPE_PTR(LightCommit, 16);

struct CascadedShadowCommit
{
	ARRAY_OF(splitDepths, float, cascade_shadow_map_cascade_count);
	uint32_t cameraID;
	float farClip;
	int index;
	uint32_t pad;
};

REGISTER_TYPE_PTR(CascadedShadowCommit, 4);

struct OmniShadowCommit
{
	uint32_t cameraID;
	float farPlane;
	int index;
	uint32_t pad;
};

REGISTER_TYPE_PTR(OmniShadowCommit, 4);

struct SpotShadowCommit
{
	uint32_t cameraID;
	int index;
};

REGISTER_TYPE_PTR(SpotShadowCommit, 4);

SHLIB_NS_END

#endif // SHLIB_SCENE_H
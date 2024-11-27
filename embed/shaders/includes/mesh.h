#ifndef SHLIB_MESH_H
#define SHLIB_MESH_H

#include "draw.h"

SHLIB_NS_BEGIN

struct GeometryDrawBounds
{
#ifdef __cplusplus
	GeometryDrawBounds() : min{ (std::numeric_limits<float>::max)() }, max{ -(std::numeric_limits<float>::max)() } {}
#endif
	ALIGN_AS(16) vec3 min;
	ALIGN_AS(16) vec3 max;
};

struct MeshLod
{
	// Default data
	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t firstVertex;
	uint32_t vertexCount;
}; // For lod buffer

REGISTER_TYPE_PTR(MeshLod, 4);

struct Mesh
{
	uint32_t materialID;

	uint32_t lodOffset;
	uint32_t lodCount;

	uint32_t pad;

	GeometryDrawBounds bounds;
};

REGISTER_TYPE_PTR(Mesh, 16);

SHLIB_NS_END

#endif // SHLIB_MESH_H
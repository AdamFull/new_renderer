#ifndef SHLIB_MESH_H
#define SHLIB_MESH_H

#include "geometry.h"

SHLIB_NS_BEGIN

struct BoundingBox
{
#ifdef __cplusplus
	BoundingBox() : min{ (std::numeric_limits<float>::max)() }, max{ -(std::numeric_limits<float>::max)() } {}
#endif
	ALIGN_AS(16) vec3 min;
	ALIGN_AS(16) vec3 max;
};

struct ALIGN_AS(4) BoundingBoxEnc
{
	i16vec3 min;
	i16vec3 max;
};

SHLIB_INLINE BoundingBoxEnc EncodeBoundingBox(BoundingBox bb, vec3 enc)
{
	BoundingBoxEnc ebb;
	ebb.min = EncodePosition(bb.min, enc);
	ebb.max = EncodePosition(bb.max, enc);
	return ebb;
}

SHLIB_INLINE BoundingBox DecodeBoundingBox(BoundingBoxEnc bb, mat4 dec)
{
	BoundingBox dbb;
	dbb.min = vec3(dec * vec4(DecodePosition(bb.min), 1.0f));
	dbb.max = vec3(dec * vec4(DecodePosition(bb.max), 1.0f));
	return dbb;
}

struct MeshLod
{
	// Default data
	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t firstVertex;
	uint32_t vertexCount;
}; // For lod buffer

REGISTER_TYPE_PTR(MeshLod, 4);

struct SubMesh
{
	uint32_t material;

	uint32_t first_index;
	uint32_t index_count;
	int32_t first_vertex;
	uint32_t vertex_count;

	//ALIGN_AS(4) BoundingBoxEnc ebb;
};

REGISTER_TYPE_PTR(SubMesh, 4);

struct Mesh
{
	uint32_t matrix_offset;
	uint32_t submesh_offset;
	uint32_t submesh_count;
	uint32_t skin_offset;

	TexcoordBounds uv_enc;
	ALIGN_AS(16) vec3 bb_enc;
	ALIGN_AS(16) BoundingBoxEnc ebb;
};

REGISTER_TYPE_PTR(Mesh, 16);

SHLIB_NS_END

#endif // SHLIB_MESH_H
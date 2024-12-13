#ifndef SHLIB_GEOMETRY_H
#define SHLIB_GEOMETRY_H

#include "base.h"

SHLIB_NS_BEGIN

// Define vertex color type dependent on element count
#if (defined(COLOR_VEC3) || defined(COLOR_VEC4))
#define HAS_COLOR 1
#else
#define HAS_COLOR 0
#endif

#if defined(COLOR_VEC4)
#define Color vec4
#define VTX_COLOR_ELEM_CNT 4
#else
#define Color vec3
#define VTX_COLOR_ELEM_CNT 3
#endif

// Define joints type
#if defined(JOINTS_16BIT)
#define JointIndex uint16_t
#define Joints u16vec4
#define JOINT_ALIGN 2
#else
#define JointIndex uint8_t
#define Joints u8vec4
#define JOINT_ALIGN 1
#endif

#if defined(INDEX_16BIT)
#define Index uint16_t
#define INDEX_ALIGN 2
#else
#define Index uint32_t
#define INDEX_ALIGN 4
#endif


// Based on https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
SHLIB_INLINE vec2 OctWrap(vec2 v)
{
	return (1.0f - abs(vec2(v.y, v.x))) * mix(vec2(-1.0f), vec2(1.0f), greaterThanEqual(v, vec2(0.0f)));
}

SHLIB_INLINE u16vec2 EncodeNormal(vec3 n)
{
	n /= abs(n.x) + abs(n.y) + abs(n.z);
	vec2 xy = vec2(n);
	xy = n.z >= 0.0f ? xy : OctWrap(xy);
	xy = xy * 0.5f + 0.5f;
	return u16vec2(min(uvec2(round(xy * float(1 << 16))), (1u << 16) - 1));
}

SHLIB_INLINE vec3 DecodeNormal(u16vec2 n)
{
	vec2 xy = vec2(n) / float(1 << 16);
	xy = xy * 2.0f - 1.0f;
	float z = 1.0f - abs(xy.x) - abs(xy.y);
	xy = z >= 0.0f ? xy : OctWrap(xy);
	return normalize(vec3(xy, z));
}

SHLIB_INLINE u16vec2 EncodeTangent(vec4 t)
{
	u16vec2 q = EncodeNormal(vec3(t.x, t.y, t.z));
	q.y = uint16_t((t.w == 1.0f) ? q.y | 1u : q.y & ~1u);
	return q;
}

SHLIB_INLINE vec4 DecodeTangent(u16vec2 t)
{
	vec3 tangent = DecodeNormal(t);
	float sign = bool(t.y & 1u) ? 1.0f : -1.0f;
	return vec4(tangent, sign);
}

SHLIB_NS_END

#endif // SHLIB_GEOMETRY_H
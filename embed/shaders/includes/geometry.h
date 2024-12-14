#ifndef SHLIB_GEOMETRY_H
#define SHLIB_GEOMETRY_H

#include "base.h"

SHLIB_NS_BEGIN

// Define joints type
#if defined(JOINTS_16BIT)
#define Joints u16vec4
#else
#define Joints u8vec4
#endif

#if defined(INDEX_16BIT)
#define Index uint16_t
#else
#define Index uint32_t
#endif

struct TexcoordBounds
{
	vec2 min;
	vec2 max;
};

SHLIB_INLINE mat4 MakeEncodePositionMatrix(vec3 bb)
{
	vec3 scale = float(1 << 15) / bb;
	mat4 m = mat4(1.0f);
	m[0][0] = scale.x;
	m[1][1] = scale.y;
	m[2][2] = scale.z;
	return m;
}

SHLIB_INLINE mat4 MakeDecodePositionMatrix(vec3 bb)
{
	vec3 scale = bb / float(1 << 15);
	mat4 m = mat4(1.0f);
	m[0][0] = scale.x;
	m[1][1] = scale.y;
	m[2][2] = scale.z;
	return m;
}

SHLIB_INLINE i16vec3 EncodePosition(vec3 p, vec3 bb)
{
	vec3 scale = float(1 << 15) / bb;
	return i16vec3(min(ivec3(round(p * scale)), (1 << 15) - 1));
}

SHLIB_INLINE vec3 DecodePosition(i16vec3 p, vec3 bb)
{
	mat4 scale = MakeDecodePositionMatrix(bb);
	return vec3(scale * vec4(p, 1));
}

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

SHLIB_INLINE u16vec2 EncodeTexcoord(vec2 uv, TexcoordBounds bounds)
{
	vec2 fuv = float(1 << 16) * (uv - bounds.min) / (bounds.max - bounds.min);
	fuv = clamp(round(fuv), 0.0f, float((1 << 16) - 1));
	return  u16vec2(fuv);
}

SHLIB_INLINE vec2 DecodeTexcoord(u16vec2 uv, TexcoordBounds bounds)
{
	return mix(bounds.min, bounds.max, vec2(uv) / float(1 << 16));
}

SHLIB_INLINE u8vec4 EncodeColor(vec4 c)
{
	return u8vec4(clamp(round(c * 255.0f), 0.0f, 255.0f));
}

SHLIB_INLINE vec4 DecodeColor(u8vec4 c)
{
	return vec4(c) / 255.0f;
}

SHLIB_NS_END

#endif // SHLIB_GEOMETRY_H
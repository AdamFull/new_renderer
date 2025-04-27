#pragma once

#ifdef __cplusplus

#include <glm/glm.hpp>
#include <glm/packing.hpp>
#include <glm/gtx/compatibility.hpp>

using int8_t2 = glm::i8vec2;
using int8_t3 = glm::i8vec3;
using int8_t4 = glm::i8vec4;

using uint8_t2 = glm::u8vec2;
using uint8_t3 = glm::u8vec3;
using uint8_t4 = glm::u8vec4;

using int16_t2 = glm::i16vec2;
using int16_t3 = glm::i16vec3;
using int16_t4 = glm::i16vec4;

using uint16_t2 = glm::u16vec2;
using uint16_t3 = glm::u16vec3;
using uint16_t4 = glm::u16vec4;

using int32_t2 = glm::i32vec2;
using int32_t3 = glm::i32vec3;
using int32_t4 = glm::i32vec4;

using uint32_t2 = glm::u32vec2;
using uint32_t3 = glm::u32vec3;
using uint32_t4 = glm::u32vec4;

using int64_t2 = glm::i64vec2;
using int64_t3 = glm::i64vec3;
using int64_t4 = glm::i64vec4;

using uint64_t2 = glm::u64vec2;
using uint64_t3 = glm::u64vec3;
using uint64_t4 = glm::u64vec4;

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;

using double2 = glm::dvec2;
using double3 = glm::dvec3;
using double4 = glm::dvec4;

using float2x2 = glm::mat2;
using float2x3 = glm::mat2x3;
using float2x4 = glm::mat2x4;

using float3x2 = glm::mat3x2;
using float3x3 = glm::mat3;
using float3x4 = glm::mat3x4;

using float4x2 = glm::mat4x2;
using float4x3 = glm::mat4x3;
using float4x4 = glm::mat4;

namespace interop {
	using namespace glm;

	template<typename _Ty>
	inline _Ty select(bool cond, _Ty l, _Ty r) {
		return cond ? l : r;
	}

	template<typename _Ty>
	struct GPUPointer {
		GPUPointer(uint64_t addr) : address{ addr } {}

		const _Ty& operator[](size_t idx) const {
			static _Ty t{};
			return t;
		};

		operator bool() const {
			return address != ~0ull;
		}

		uint64_t address{ ~0ull };
	};
}

#define TYPE_PTR(type) interop::GPUPointer<type>
#define CONSTRUCTOR(type, ...) type(__VA_ARGS__)
#elif __SLANG__
#define TYPE_PTR(type) type*
#define CONSTRUCTOR(type, ...) __init(__VA_ARGS__)
#endif

#if defined(JOINTS_16BIT)
typedef uint16_t4 jounts_t;
#else
typedef uint8_t4 jounts_t;
#endif

#if defined(INDEX_16BIT)
typedef uint16_t sindex_t;
#else
typedef uint32_t sindex_t;
#endif

namespace interop {
	struct DrawIndexedIndirectData {
		uint32_t index_count;
		uint32_t instance_count;
		uint32_t first_index;
		int32_t  vertex_offset;
		uint32_t first_instance;
	};

	struct DrawDescription {
		uint32_t camera_index;
		uint32_t mesh_index;
		uint32_t submesh_index;
		uint32_t sorting_order;
	};

	struct TexcoordBounds {
		float2 min;
		float2 max;
	};

	inline float4x4 make_encode_position_matrix(float3 bb) {
		float3 scale = float(1 << 15) / bb;
		float4x4 m = float4x4(1.0f);
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		m[2][2] = scale.z;
		return m;
	}

	inline float4x4 make_decode_position_matrix(float3 bb) {
		float3 scale = bb / float(1 << 15);
		float4x4 m = float4x4(1.0f);
		m[0][0] = scale.x;
		m[1][1] = scale.y;
		m[2][2] = scale.z;
		return m;
	}

	inline int16_t3 encode_position(float3 p, float3 bb) {
		float3 scale = float(1 << 15) / bb;
		return int16_t3(min(int32_t3(round(p * scale)), (1 << 15) - 1));
	}

	inline float3 decode_position(int16_t3 p) {
		return float3(p);
	}

	// Based on https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
	inline float2 oct_wrap(float2 v) {
		return (float2(1.0f) - abs(v)) * (select(v.x >= 0.0f && v.y >= 0.0f, 1.0f, -1.0f));
	}

	inline uint16_t2 encode_normal(float3 n) {
		n /= abs(n.x) + abs(n.y) + abs(n.z);
		float2 xy = float2(n.x, n.y);
		xy = select(n.z >= 0.0f, xy, oct_wrap(xy));
		xy = xy * 0.5f + 0.5f;
		return uint16_t2(min(uint32_t2(round(xy * float(1 << 16))), uint32_t2((1u << 16) - 1)));
	}

	inline float3 decode_normal(uint16_t2 n) {
		float2 xy = float2(n) / float(1 << 16);
		xy = xy * 2.0f - 1.0f;
		float z = 1.0f - abs(xy.x) - abs(xy.y);
		xy = select(z >= 0.0f, xy, oct_wrap(xy));
		return normalize(float3(xy, z));
	}

	inline uint16_t2 encode_tangent(float4 t) {
		uint16_t2 q = encode_normal(float3(t.x, t.y, t.z));
		q.y = uint16_t(select(t.w == 1.0f, q.y | 1u, q.y & ~1u));
		return q;
	}

	inline float4 decode_tangent(uint16_t2 t) {
		float3 tangent = decode_normal(t);
		float sign = select(bool(t.y & 1u), 1.0f, -1.0f);
		return float4(tangent, sign);
	}

	inline uint16_t2 encode_texcoord(float2 uv, TexcoordBounds bounds) {
		float2 fuv = float(1 << 16) * (uv - bounds.min) / (bounds.max - bounds.min);
		fuv = clamp(round(fuv), 0.0f, float((1 << 16) - 1));
		return  uint16_t2(fuv);
	}

	inline float2 decode_texcoord(uint16_t2 uv, TexcoordBounds bounds) {
		return lerp(bounds.min, bounds.max, float2(uv) / float(1 << 16));
	}

	inline uint8_t4 encode_color(float4 c) {
		return uint8_t4(clamp(round(c * 255.0f), 0.0f, 255.0f));
	}

	inline float4 decode_color(uint8_t4 c) {
		return float4(c) / 255.0f;
	}

	inline uint16_t3 encode_weights(float3 w) {
		uint16_t w0_u16 = uint16_t(w.x * 65535.0f + 0.5f);
		uint16_t w1_u16 = uint16_t(w.y * 65535.0f + 0.5f);
		uint16_t w2_u16 = uint16_t(w.z * 65535.0f + 0.5f);

		return uint16_t3(w0_u16, w1_u16, w2_u16);
	}

	inline float4 decode_weights(uint16_t3 w) {
		float4 weights;

		weights.x = float(w.x) / 65535.0;
		weights.y = float(w.y) / 65535.0;
		weights.z = float(w.z) / 65535.0;

		// Compute w.w so that sum equals 1.0
		weights.w = 1.0 - (weights.x + weights.y + weights.z);

		return weights;
	}

	struct EncodedVertex {
		int16_t3 position;
		uint8_t4 color;
		uint16_t2 uv;
		uint16_t2 normal;
		uint16_t2 tangent;

		jounts_t joints;
		uint16_t3 weights;
	};

	struct DecodedVertex {
		float3 position;
		float3 normal;
		float4 tangent;
		float2 uv;
		float4 color;

		jounts_t joints;
		float4 weights;

		CONSTRUCTOR(DecodedVertex, EncodedVertex enc_vert, TexcoordBounds uv_bb) {
			position = decode_position(enc_vert.position);
			normal = decode_normal(enc_vert.normal);
			tangent = decode_tangent(enc_vert.tangent);
			uv = decode_texcoord(enc_vert.uv, uv_bb);
			color = decode_color(enc_vert.color);
			joints = enc_vert.joints;
			weights = decode_weights(enc_vert.weights);
		}
	};

	struct EncodedGeometryData {
		TYPE_PTR(int16_t3) position;
		TYPE_PTR(uint8_t4) color;
		TYPE_PTR(uint16_t2) texcoord;
		TYPE_PTR(uint16_t2) normal;
		TYPE_PTR(uint16_t2) tangent;

		TYPE_PTR(jounts_t) joints;
		TYPE_PTR(uint16_t3) weights;

		TYPE_PTR(sindex_t) indices;

		inline EncodedVertex get_enc_vertex(uint32_t vertex_index) {
			EncodedVertex enc_vert;

			if (position)
				enc_vert.position = position[vertex_index];
			if (color)
				enc_vert.color = color[vertex_index];
			if (texcoord)
				enc_vert.uv = texcoord[vertex_index];
			if (normal)
				enc_vert.normal = normal[vertex_index];
			if (tangent)
				enc_vert.tangent = tangent[vertex_index];

			if (joints)
				enc_vert.joints = joints[vertex_index];
			if (weights)
				enc_vert.weights = weights[vertex_index];

			return enc_vert;
		}

		inline DecodedVertex get_dec_vertex(uint32_t vertex_index, TexcoordBounds uv_bb) {
			EncodedVertex enc_vert = get_enc_vertex(vertex_index);
			return DecodedVertex(enc_vert, uv_bb);
		}
	};
}
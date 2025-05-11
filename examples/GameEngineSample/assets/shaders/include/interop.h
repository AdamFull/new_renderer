#pragma once

#include "constants.h"

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

	template<typename _Ty, typename _Tty>
	inline auto mul(const _Ty& a, const _Tty& b) -> decltype(a * b) {
		return a * b;
	}

	template<typename _Ty>
	struct GPUPointer {
		GPUPointer() = default;
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
	struct TexcoordBounds {
		float2 min;
		float2 max;
	};

	inline int16_t3 encode_position(float3 p, float3 bb) {
		float3 scale = float(1 << 15) / bb;
		return int16_t3(min(int32_t3(round(p * scale)), (1 << 15) - 1));
	}

	inline float3 decode_position(int16_t3 p) {
		return float3(p);
	}

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

	// Based on https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
	inline float2 oct_wrap(float2 v) {
		const float2 t = 1.0f - abs(float2(v.y, v.x));
		return float2(v.x < 0.0f ? -t.x : t.x, v.y < 0.0f ? -t.y : t.y);
	}

	inline float3 ortho_vec(float3 v) {
		if (abs(v.y) > abs(v.z)) {
			return float3(v.y, -v.x, 0.0f);
		}
		return float3(v.z, 0.0f, -v.x);
	}

	inline float sq_wrap(float v) {
		return (2.0f - abs(v)) * (v >= 0.0f ? 1.0f : -1.0f);
	}

	inline float3 decode_normal(uint16_t2 n) {
		float2 xy = float2(n) / float(1 << 16);
		xy = xy * 2.0f - 1.0f;
		float z = 1.0f - abs(xy.x) - abs(xy.y);
		xy = select(z >= 0.0f, xy, oct_wrap(xy));
		return normalize(float3(xy, z));
	}

	inline float4 decode_tangent(uint16_t t, float3 n) {
		float3 t1 = normalize(ortho_vec(n));
		float3 t2 = cross(n, t1);
		float x = (t & ((1 << 15) - 1)) / float(1 << 15);
		x = x * 4.0f - 2.0f;
		float y = 1.0f - abs(x);
		x = y >= 0.0f ? x : sq_wrap(x);
		float2 xy = normalize(float2(x, y));
		float sign = bool(t & (1 << 15)) ? -1.0f : 1.0f;
		return float4(t1 * xy.x + t2 * xy.y, sign);
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

	struct BoundingBox {
		CONSTRUCTOR(BoundingBox) {
			min = float3(constants::flt_max);
			_pad0 = 0.0f;
			max = float3(-constants::flt_max);
			_pad1 = 0.0f;
		}

		CONSTRUCTOR(BoundingBox, float3 min_, float3 max_) {
			min = min_;
			_pad0 = 0.0f;
			max = max_;
			_pad1 = 0.0f;
		}

		float3 min;
		float _pad0;
		float3 max;
		float _pad1;
	};

	struct BoundingBoxEnc {
		CONSTRUCTOR(BoundingBoxEnc) {
			min = int16_t3(constants::flt_max);
			_pad0 = 0;
			max = int16_t3(-constants::flt_max);
			_pad1 = 0;
		}

		CONSTRUCTOR(BoundingBoxEnc, BoundingBox bb, float3 enc) {
			min = encode_position(bb.min, enc);
			_pad0 = 0;
			max = encode_position(bb.max, enc);
			_pad1 = 0;
		}

		BoundingBox decode(float4x4 dec) {
			float4 mn = mul(dec, float4(decode_position(min), 1.0f));
			float4 mx = mul(dec, float4(decode_position(max), 1.0f));
			return BoundingBox(float3(mn.x, mn.y, mn.z) / mn.w, float3(mx.x, mx.y, mx.z) / mx.w);
		}

		int16_t3 min;
		uint16_t _pad0;
		int16_t3 max;
		uint16_t _pad1;
	};

	inline BoundingBoxEnc encode_bounding_box(BoundingBox bb, float3 enc) {
		return BoundingBoxEnc(bb, enc);
	}

	struct MeshLod {
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t firstVertex;
		uint32_t vertexCount;
	};

	struct SubMesh {
		uint32_t material;

		uint32_t first_index;
		uint32_t index_count;
		int32_t first_vertex;
		uint32_t vertex_count;
		uint32_t _pad0;
		uint32_t _pad1;
		uint32_t _pad2;

		BoundingBoxEnc ebb;
	};

	struct Mesh {
		uint32_t matrix_offset;
		uint32_t submesh_offset;
		uint32_t submesh_count;
		uint32_t skin_offset;

		TexcoordBounds uv_enc;
		float3 bb_enc;
		float _pad0;
		BoundingBoxEnc ebb;
	};

	enum class AlphaMode {
		eBlend,
		eMask, 
		eOpaque
	};

	struct Texture {
		CONSTRUCTOR(Texture) {
			texture = constants::invalid_index;
			sampler = constants::invalid_index;
		}

		bool valid() {
			return texture != constants::invalid_index;
		}

		uint16_t texture;
		uint16_t sampler;
	};

	struct Material {
		Texture base_color;
		Texture normal;
		Texture metallic_roughness;
		Texture occlusion;

		float3 emissive_factor = float3(0.0f, 0.0f, 0.0f);
		Texture emissive;

		float4 base_color_factor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		float2 metal_rough_factor = float2(1.0f, 1.0f);
		float alpha_cutoff = 0.5f;
		AlphaMode alpha_mode = AlphaMode::eOpaque;
	};

	enum class LightType {
		eInvalid,
		eDirectional,
		ePoint,
		eSpot
	};

	struct LightCommit {
		float3 position;
		float intencity;
		float3 direction;
		float light_angle_scale;
		float3 color;
		float light_angle_offset;
		int to_target;
		float radius;
		int shadow_index;
		int cast_shadows;
		LightType type;
		int _pad0;
	};

	struct CascadedShadowCommit {
		float split_depths[constants::cascade_shadow_map_cascade_count];
		uint32_t matrix_offsets[constants::cascade_shadow_map_cascade_count];
		float far_clip;
		int index;
		uint32_t pad;
	};

	struct OmniShadowCommit {
		uint32_t matrix_offsets[6];
		float far_plane;
		int index;
	};

	struct SpotShadowCommit {
		uint32_t matrix_offset;
		int index;
	};

	struct Camera {
		uint32_t type;
		uint32_t matrices_offset;
		uint32_t frustum_offset;
		uint32_t last_hash;

		float3 position;
		float _pad0;

		float fov;
		float z_near;
		float z_far;
		float aspect;

		float2 size;
		float2 inv_size;
	};

	struct FrameData {
		TYPE_PTR(float4x4) matrix_ptr;
		TYPE_PTR(float4) frustum_sides_ptr;
		TYPE_PTR(Mesh) mesh_ptr;
		TYPE_PTR(SubMesh) submesh_ptr;
		TYPE_PTR(Camera) camera_ptr;
		TYPE_PTR(Material) material_ptr;
		TYPE_PTR(LightCommit) lights_ptr;
		TYPE_PTR(CascadedShadowCommit) cascaded_shadow_ptr;
		TYPE_PTR(OmniShadowCommit) omni_shadow_ptr;
		TYPE_PTR(SpotShadowCommit) spot_shadow_ptr;

		float dt;
		uint32_t directional_light_count;
		uint32_t point_light_count;
		uint32_t spot_light_count;
		uint32_t light_count;
		uint32_t camera_count;
		uint32_t draw_count;
		uint32_t mesh_count;
		uint32_t default_camera_id;
		uint32_t frame_number;

		inline Camera get_default_camera() {
			return camera_ptr[default_camera_id];
		}

		inline float4x4 get_model_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::model_matrix_offset];
		}
		inline float4x4 get_old_model_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::old_model_matrix_offset];
		}
		inline float4x4 get_normal_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::normal_matrix_offset];
		}

		inline float4x4 get_view_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::view_matrix_offset];
		}
		inline float4x4 get_inv_view_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::inv_view_matrix_offset];
		}
		inline float4x4 get_projection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::projection_matrix_offset];
		}
		inline float4x4 get_inv_projection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::inv_projection_matrix_offset];
		}
		inline float4x4 get_view_projection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::view_projection_matrix_offset];
		}
		inline float4x4 get_prev_view_projection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::prev_view_projection_matrix_offset];
		}
		inline float4x4 get_inv_view_projection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::inv_view_projection_matrix_offset];
		}
		inline float4x4 get_reprojection_matrix(uint global_offset) { 
			return matrix_ptr[global_offset + constants::reprojection_matrix_offset];
		}

	};

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

	struct RawVertex {
		float3 position;
		float3 normal;
		float4 tangent;
		float4 color;
		float2 uv;
	};

	struct Vertex {
		int16_t3 position;
		uint16_t tangent;
		uint16_t2 normal;
		uint16_t2 uv;
		uint8_t4 color;
	};

	struct Triangle {
		RawVertex vertices[3];
	};

	struct EncodedGeometryData {
		TYPE_PTR(Vertex) vertices;
		TYPE_PTR(sindex_t) indices;

		TYPE_PTR(DrawDescription) draw_descriptions_ptr;
		TYPE_PTR(DrawIndexedIndirectData) draw_data_ptr;

		inline RawVertex get_vertex(uint32_t vertex_index, TexcoordBounds uv_bb) {
			Vertex enc_vertex = vertices[vertex_index];

			RawVertex vertex;
			vertex.position = decode_position(enc_vertex.position);
			vertex.normal = decode_normal(enc_vertex.normal);
			vertex.tangent = decode_tangent(enc_vertex.tangent, vertex.normal);
			vertex.uv = decode_texcoord(enc_vertex.uv, uv_bb);
			return vertex;
		}

		inline Triangle get_triangle(uint32_t draw_id, uint32_t prim_id, TexcoordBounds uv_bb) {
			Triangle triangle;

			DrawIndexedIndirectData draw_command = draw_data_ptr[draw_id];

			// Index of the first vertex of this Draw call's geometry
			uint32_t start_index = draw_command.first_index;

			// Get position in the Index Buffer of each vertex in this triangle (eg 31, 32, 33)
			uint32_t tri_vert1_index_buffer_position = (prim_id * 3 + 0) + start_index;
			uint32_t tri_vert2_index_buffer_position = (prim_id * 3 + 1) + start_index;
			uint32_t tri_vert3_index_buffer_position = (prim_id * 3 + 2) + start_index;

			// Now get vertex index from index buffer (eg, 17, 41, 32)
			sindex_t tri_vert0_index = indices[tri_vert1_index_buffer_position];
			sindex_t tri_vert1_index = indices[tri_vert2_index_buffer_position];
			sindex_t tri_vert2_index = indices[tri_vert3_index_buffer_position];

			triangle.vertices[0] = get_vertex(tri_vert0_index + draw_command.vertex_offset, uv_bb);
			triangle.vertices[1] = get_vertex(tri_vert1_index + draw_command.vertex_offset, uv_bb);
			triangle.vertices[2] = get_vertex(tri_vert2_index + draw_command.vertex_offset, uv_bb);

			return triangle;
		}
	};
}
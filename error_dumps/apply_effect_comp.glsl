Preprocessing error: In shader "apply_effect_comp" 
WARNING: apply_effect_comp:14: '#define' : names containing consecutive underscores are reserved: __glslang
ERROR: apply_effect_comp:1106: '#' : invalid directive: definde
ERROR: apply_effect_comp:1106: '' : missing #endif 
ERROR: 2 compilation errors.  No code generated.



#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_shader_draw_parameters : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_shader_16bit_storage : enable

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

#define __glslang
#define COMPUTE_SHADER




#ifndef compute_common_protector
#define compute_common_protector

		uint ABfiM(uint src, uint ins, uint bits)
		{
			uint mask = (1 << bits) - 1;
			return (ins & mask) | (src & (~mask));
		}
		
		uint ABfe(uint src, uint off, uint bits)
		{
			uint mask = (1<<bits) - 1;
			return (src >> off) & mask;
		}
		
		uvec2 ARmp8x8(uint a)
		{
			return uvec2(ABfe(a, 1, 3), ABfiM(ABfe(a, 3, 3), a, 1));
		}

		// https://github.com/LouisBavoil/ThreadGroupIDSwizzling/blob/master/ThreadGroupTilingX.hlsl
		// Divide the 2D-Dispatch_Grid into tiles of dimension [N, DipatchGridDim.y]
		// “CTA” (Cooperative Thread Array) == Thread Group in DirectX terminology
		uvec2 ThreadGroupTilingX(
			const uvec2 dipatchGridDim,		// Arguments of the Dispatch call (typically from a ConstantBuffer)
			const uvec2 ctaDim,			// Already known in HLSL, eg:[numthreads(8, 8, 1)] -> uvec2(8, 8)
			const uint maxTileWidth,		// User parameter (N). Recommended values: 8, 16 or 32.
			const uvec2 groupThreadID,		// SV_GroupThreadID
			const uvec2 groupId			// SV_GroupID
		)
		{
			// A perfect tile is one with dimensions = [maxTileWidth, dipatchGridDim.y]
			const uint Number_of_CTAs_in_a_perfect_tile = maxTileWidth * dipatchGridDim.y;
		
			// Possible number of perfect tiles
			const uint Number_of_perfect_tiles = dipatchGridDim.x / maxTileWidth;
		
			// Total number of CTAs present in the perfect tiles
			const uint Total_CTAs_in_all_perfect_tiles = Number_of_perfect_tiles * maxTileWidth * dipatchGridDim.y;
			const uint vThreadGroupIDFlattened = dipatchGridDim.x * groupId.y + groupId.x;
		
			// Tile_ID_of_current_CTA : current CTA to TILE-ID mapping.
			const uint Tile_ID_of_current_CTA = vThreadGroupIDFlattened / Number_of_CTAs_in_a_perfect_tile;
			const uint Local_CTA_ID_within_current_tile = vThreadGroupIDFlattened % Number_of_CTAs_in_a_perfect_tile;
			uint Local_CTA_ID_y_within_current_tile;
			uint Local_CTA_ID_x_within_current_tile;
		
			if (Total_CTAs_in_all_perfect_tiles <= vThreadGroupIDFlattened)
			{
				// Path taken only if the last tile has imperfect dimensions and CTAs from the last tile are launched. 
				uint X_dimension_of_last_tile = dipatchGridDim.x % maxTileWidth;
			#ifdef DXC_STATIC_DISPATCH_GRID_DIM
				X_dimension_of_last_tile = max(1, X_dimension_of_last_tile);
			#endif
				Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / X_dimension_of_last_tile;
				Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % X_dimension_of_last_tile;
			}
			else
			{
				Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / maxTileWidth;
				Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % maxTileWidth;
			}
		
			const uint Swizzled_vThreadGroupIDFlattened =
				Tile_ID_of_current_CTA * maxTileWidth +
				Local_CTA_ID_y_within_current_tile * dipatchGridDim.x +
				Local_CTA_ID_x_within_current_tile;
		
			uvec2 SwizzledvThreadGroupID;
			SwizzledvThreadGroupID.y = Swizzled_vThreadGroupIDFlattened / dipatchGridDim.x;
			SwizzledvThreadGroupID.x = Swizzled_vThreadGroupIDFlattened % dipatchGridDim.x;
		
			uvec2 SwizzledvThreadID;
			SwizzledvThreadID.x = ctaDim.x * SwizzledvThreadGroupID.x + groupThreadID.x;
			SwizzledvThreadID.y = ctaDim.y * SwizzledvThreadGroupID.y + groupThreadID.y;
		
			return SwizzledvThreadID.xy;
		}

		bool swizzle_workgroups_8x8(in uvec2 gid, in uvec2 lid, in uvec2 wid, in uvec2 face_size, out vec2 uv, out ivec2 px)
		{
//#if defined(VENDOR_NVIDIA)
//			const uvec2 cta = uvec2(8, 8);
//			const uvec2 grid_dim = face_size / cta;
//			px = ivec2(ThreadGroupTilingX(grid_dim, cta, 8, lid, wid));
//#elif defined(VENDOR_AMD)
			uvec2 base_tx = gid - lid;
			uint lid1d = lid.y * 8 + lid.x;
			uvec2 sub_xy = ARmp8x8(lid1d);
			uvec2 tx = uvec2(sub_xy.x + 8 * ((lid1d >> 6) % 2), sub_xy.y + 8 * ((lid1d >> 7)));
			px = ivec2(base_tx + tx);
//#else
//			px = ivec2(gid);
//#endif
			
			uv = (vec2(px) + vec2(0.5, 0.5)) / vec2(face_size);
			if (px.x > face_size.x || px.y > face_size.y)
				return true;
		
			return false;
		}
	
#endif


#ifndef shader_inputs_protector
#define shader_inputs_protector

#ifndef INTEROP_BASE_H
#define INTEROP_BASE_H

#ifdef __cplusplus

#include <glm/glm.hpp>
#include <standart_library.h>

#define INVALID_HANDLE ~0ull

#define ALIGN_AS(x) alignas(x)

#define ARRAY_OF(name, type, count) std::array<type, count> name

#define CONSTANT constexpr const

#define VECTOR2 glm::vec2
#define VECTOR3 glm::vec3
#define VECTOR4 glm::vec4
#define MATRIX2 glm::mat2
#define MATRIX3 glm::mat3
#define MATRIX4 glm::mat4

#define REGISTER_OBJECT_ARRAY_BUFFER(BufferTypeName, DataType, ArrayName) typedef uint64_t BufferTypeName
#else // __cplusplus

#define INVALID_HANDLE ~0u

#define ALIGN_AS(x)

#define ARRAY_OF(name, type, count) type name[count]

#ifdef __glslang
#define CONSTANT const

#define VECTOR2 vec2
#define VECTOR3 vec3
#define VECTOR4 vec4
#define MATRIX2 mat2
#define MATRIX3 mat3
#define MATRIX4 mat4

#define REGISTER_OBJECT_ARRAY_BUFFER(BufferTypeName, DataType, ArrayName) layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer BufferTypeName { DataType ArrayName[]; };
#else // __glslang
#define CONSTANT static const

#define VECTOR2 float2
#define VECTOR3 float3
#define VECTOR4 float4
#define MATRIX2 float2x2
#define MATRIX3 float3x3
#define MATRIX4 float4x4

#define REGISTER_OBJECT_ARRAY_BUFFER(BufferTypeName, DataType, ArrayName) \
		struct F##BufferTypeName##_Array{ DataType ArrayName[]; }; \
		typedef vk::BufferPointer<F##BufferTypeName##_Array> BufferTypeName
#endif // __glslang
#endif // __cplusplus

#ifdef __cplusplus
namespace engine { 
	namespace render {
#endif
		CONSTANT uint32_t kShaderGroupSize = 32u;
		CONSTANT uint32_t kMaxVerticesPerMeshlet = 64u;
		CONSTANT uint32_t kMaxTrianglesPerMeshlet = 124u;
		CONSTANT uint32_t kMaxMeshLodCount = 4u;

		// Directional light
		CONSTANT uint32_t max_directional_light_count = 3u;
		CONSTANT uint32_t max_directional_light_shadow_count = 1u;
		CONSTANT uint32_t cascade_shadow_map_resolution = 2048u;
		CONSTANT uint32_t cascade_shadow_map_cascade_count = 4u;

		// Spot light
		CONSTANT uint32_t max_spot_light_count = 64u;
		CONSTANT uint32_t max_spot_light_shadow_count = 1u;
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

#if defined(INDEX_8BIT)
#define Index uint8_t
#elif defined(INDEX_16BIT)
#define Index uint16_t
#else
#define Index uint
#endif

		struct FRawVertex
		{
			float px;
			float py;
			float pz;

#if defined(COLOR_VEC3) || defined(COLOR_VEC4)
			float cr;
			float cg;
			float cb;
#endif // COLOR_VEC3
#if defined(COLOR_VEC4)
			float ca;
#endif // COLOR_VEC4

#ifdef HAS_TEXCOORD
			float u;
			float v;
#endif // HAS_TEXCOORD

#ifdef HAS_NORMAL
			float nx;
			float ny;
			float nz;
#endif // HAS_NORMAL

#if defined(TANGENT_VEC3) || defined(TANGENT_VEC4)
			float tx;
			float ty;
			float tz;
#endif // TANGENT_VEC3
#if defined(TANGENT_VEC4)
			float tw;
#endif // TANGENT_VEC4

#ifdef HAS_SKIN
#if defined(JOINTS_8BIT) || defined(JOINTS_16BIT)
			uint8_t joints_8;
#endif // JOINTS_8BIT
#if defined(JOINTS_16BIT)
			uint16_t joints_16;
#endif // JOINTS_16BIT

			float wx;
			float wy;
			float wz;
			float ww;
#endif // HAS_SKIN
		};

#ifdef __cplusplus
		struct FFrustum
		{
		public:
			void update(const glm::mat4& view, const glm::mat4& projection)
			{
				clip[0] = view[0][0] * projection[0][0] + view[0][1] * projection[1][0] + view[0][2] * projection[2][0] + view[0][3] * projection[3][0];
				clip[1] = view[0][0] * projection[0][1] + view[0][1] * projection[1][1] + view[0][2] * projection[2][1] + view[0][3] * projection[3][1];
				clip[2] = view[0][0] * projection[0][2] + view[0][1] * projection[1][2] + view[0][2] * projection[2][2] + view[0][3] * projection[3][2];
				clip[3] = view[0][0] * projection[0][3] + view[0][1] * projection[1][3] + view[0][2] * projection[2][3] + view[0][3] * projection[3][3];

				clip[4] = view[1][0] * projection[0][0] + view[1][1] * projection[1][0] + view[1][2] * projection[2][0] + view[1][3] * projection[3][0];
				clip[5] = view[1][0] * projection[0][1] + view[1][1] * projection[1][1] + view[1][2] * projection[2][1] + view[1][3] * projection[3][1];
				clip[6] = view[1][0] * projection[0][2] + view[1][1] * projection[1][2] + view[1][2] * projection[2][2] + view[1][3] * projection[3][2];
				clip[7] = view[1][0] * projection[0][3] + view[1][1] * projection[1][3] + view[1][2] * projection[2][3] + view[1][3] * projection[3][3];

				clip[8] = view[2][0] * projection[0][0] + view[2][1] * projection[1][0] + view[2][2] * projection[2][0] + view[2][3] * projection[3][0];
				clip[9] = view[2][0] * projection[0][1] + view[2][1] * projection[1][1] + view[2][2] * projection[2][1] + view[2][3] * projection[3][1];
				clip[10] = view[2][0] * projection[0][2] + view[2][1] * projection[1][2] + view[2][2] * projection[2][2] + view[2][3] * projection[3][2];
				clip[11] = view[2][0] * projection[0][3] + view[2][1] * projection[1][3] + view[2][2] * projection[2][3] + view[2][3] * projection[3][3];

				clip[12] = view[3][0] * projection[0][0] + view[3][1] * projection[1][0] + view[3][2] * projection[2][0] + view[3][3] * projection[3][0];
				clip[13] = view[3][0] * projection[0][1] + view[3][1] * projection[1][1] + view[3][2] * projection[2][1] + view[3][3] * projection[3][1];
				clip[14] = view[3][0] * projection[0][2] + view[3][1] * projection[1][2] + view[3][2] * projection[2][2] + view[3][3] * projection[3][2];
				clip[15] = view[3][0] * projection[0][3] + view[3][1] * projection[1][3] + view[3][2] * projection[2][3] + view[3][3] * projection[3][3];

				//Left
				frustumSides[1][0] = clip[3] - clip[0];
				frustumSides[1][1] = clip[7] - clip[4];
				frustumSides[1][2] = clip[11] - clip[8];
				frustumSides[1][3] = clip[15] - clip[12];
				normalizeFrustumSide(1);

				//Right
				frustumSides[0][0] = clip[3] + clip[0];
				frustumSides[0][1] = clip[7] + clip[4];
				frustumSides[0][2] = clip[11] + clip[8];
				frustumSides[0][3] = clip[15] + clip[12];
				normalizeFrustumSide(0);

				//Top
				frustumSides[3][0] = clip[3] - clip[1];
				frustumSides[3][1] = clip[7] - clip[5];
				frustumSides[3][2] = clip[11] - clip[9];
				frustumSides[3][3] = clip[15] - clip[13];
				normalizeFrustumSide(3);

				//Bottom
				frustumSides[2][0] = clip[3] + clip[1];
				frustumSides[2][1] = clip[7] + clip[5];
				frustumSides[2][2] = clip[11] + clip[9];
				frustumSides[2][3] = clip[15] + clip[13];
				normalizeFrustumSide(2);

				//Back
				frustumSides[4][0] = clip[3] + clip[2];
				frustumSides[4][1] = clip[7] + clip[6];
				frustumSides[4][2] = clip[11] + clip[10];
				frustumSides[4][3] = clip[15] + clip[14];
				normalizeFrustumSide(4);

				//Front
				frustumSides[5][0] = clip[3] - clip[2];
				frustumSides[5][1] = clip[7] - clip[6];
				frustumSides[5][2] = clip[11] - clip[10];
				frustumSides[5][3] = clip[15] - clip[14];
			}

			/**
			 * @brief Check is point in frustum
			 *
			 * @param pos Point position
			 * @return true when point is in frustum
			 */
			bool checkPoint(const glm::vec3& pos) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * pos.x + frustumSides[i].y * pos.y + frustumSides[i].z * pos.z + frustumSides[i].w <= 0.0f)
						return false;
				}
				return true;
			}

			/**
			 * @brief Check is sphere intersects with frustum
			 *
			 * @param pos Sphere center position
			 * @param radius Sphere radius
			 * @return true when sphere intersects with camera frustum
			 */
			bool checkSphere(const glm::vec3& pos, f32 radius) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * pos.x + frustumSides[i].y * pos.y + frustumSides[i].z * pos.z + frustumSides[i].w <= -radius)
						return false;
				}
				return true;
			}

			/**
			 * @brief Check is bounding box intersects with frustum
			 *
			 * @param start Bounding box start point
			 * @param end Bounding box end boint
			 * @return true when bounding box intersects with camera frustum
			 */
			bool checkBox(const glm::vec3& start, const glm::vec3& end) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * start.x + frustumSides[i].y * start.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * start.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * end.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * end.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * start.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * start.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * end.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * end.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f)
						return false;
				}
				return true;
			}

			std::array<glm::vec4, 6> frustumSides{};
			std::array<f32, 16> clip;
		private:
			void normalizeFrustumSide(u64 side)
			{
				auto magnitude = std::sqrt(frustumSides[side][0] * frustumSides[side][0] + frustumSides[side][1] * frustumSides[side][1] + frustumSides[side][2] * frustumSides[side][2]);
				frustumSides[side][0] /= magnitude;
				frustumSides[side][1] /= magnitude;
				frustumSides[side][2] /= magnitude;
				frustumSides[side][3] /= magnitude;
			}
		};
#endif

		struct FCamera
		{
			uint32_t matricesOffset;
			uint32_t type;
			
			float zNear;
			float zFar;

			VECTOR2 size;
			VECTOR2 invSize;
		};

		struct FGeometryDrawBounds
		{
#ifdef __cplusplus
			FGeometryDrawBounds() : min{ (std::numeric_limits<float>::max)() }, max{ -(std::numeric_limits<float>::max)() } {}
#endif
			VECTOR3 min;
			uint32_t pad0;
			VECTOR3 max;
			uint32_t pad1;
		};

		struct FMeshLod
		{
			// Meshlet data
			//uint32_t meshletOffset;
			//uint32_t meshletVertexOffset;
			//uint32_t meshletTriangleOffset;
			//uint32_t meshletBoundsOffset;
			//uint32_t meshletCount;

			// Default data
			uint32_t firstIndex;
			uint32_t indexCount;
			uint32_t firstVertex; 
			uint32_t vertexCount;

			//uint32_t pad[7];
		}; // For lod buffer

		struct FMesh
		{
			uint32_t materialID;

			uint32_t lodOffset;
			uint32_t lodCount;

			uint32_t pad;

			FGeometryDrawBounds bounds;
		};

		struct FMeshlet
		{
			uint32_t vertexOffset;
			uint32_t triangleOffset;
			uint32_t vertexCount;
			uint32_t triangleCount;
		};

		struct FTriangle
		{
			uint32_t V0;
			uint32_t V1;
			uint32_t V2;
			uint32_t pad;
		}; 

		// Verticea range
		struct FGeometryDrawRange
		{
			uint32_t beginIndex;
			uint32_t indexCount;
			uint32_t beginVertex;
			uint32_t vertexCount;

			uint32_t meshletCount;
			uint32_t firstMeshlet;
		};

		struct FSceneNode
		{
			uint32_t matrixOffset;
			uint32_t skeletonID;
		};

		struct FClusterInfo
		{
			uint32_t drawID; // Index of FDrawMeshinfo
			uint32_t firstMeshlet;
			uint32_t meshletCount;
			uint32_t pad;
		};

		struct FVertexRangeData
		{
			uint32_t vertexCount;
			uint32_t instanceCount;
			uint32_t firstVertex;
			uint32_t firstInstance;
		};

		struct FDrawIndexedIndirectData
		{
			uint32_t indexCount;
			uint32_t instanceCount;
			uint32_t firstIndex;
			int32_t  vertexOffset;
			uint32_t firstInstance;
		};

		struct FDrawMeshIndirectData
		{
			uint32_t taskCount;
			uint32_t firstTask;
		};

		struct FGeometryBatch
		{
			uint32_t vboID;
			uint32_t meshID;
			uint32_t nodeID;
			uint32_t cameraID;
		};

		// Lights and shadows
		struct FCascadedShadowCommit
		{
			ARRAY_OF(splitDepths, float, cascade_shadow_map_cascade_count);
			uint32_t cameraID;
			float farClip;
			int index;
			uint32_t pad;
		};

		struct FLightCommit
		{
			ALIGN_AS(16) VECTOR3 position;
			ALIGN_AS(16) VECTOR3 direction;
			ALIGN_AS(16) VECTOR3 color;
			float intencity;
			float lightAngleScale;
			float lightAngleOffset;
			int toTarget;
			float radius;
			int shadowIndex;
			int castShadows;
			int type;
		};

		struct FOmniShadowCommit
		{
			uint32_t cameraID;
			float farPlane;
			int index;
			uint32_t pad;
		};

		struct FSpotShadowCommit
		{
			uint32_t cameraID;
			int index;
		};

		// Material
		struct FMaterial
		{
#ifdef __cplusplus
			FMaterial() :
				alphaMode{}, alphaCutoff{ 0.5f },
				baseColorTexture{ ~0u }, baseColorFactor{ 1.0f },
				metallicRoughnessTexture{ ~0u }, metallicFactor{ 1.f }, roughnessFactor{ 1.f },
				normalTexture{ ~0u }, normalScale{ 1.f },
				occlusionTexture{ ~0u }, occlusionStrenth{ 1.f },
				emissiveTexture{ ~0u }, emissiveFactor{ 0.f },
				doubleSided{ 0u },
				// KHR_materials_clearcoat
				clearcoatFactor{ 0.f }, clearcoatRoughnessFactor{ 0.f },
				// KHR_materials_emissive_strength
				emissiveStrength{ 1.f },
				// KHR_materials_ior
				ior{ 1.5f },
				// KHR_materials_iridescence
				iridescenceFactor{ 0.f }, iridescenceIor{ 1.3f }, iridescenceThicknessMinimum{ 100.f }, iridescenceThicknessMaximum{ 400.f },
				// KHR_materials_sheen
				sheenColorFactor{ 0.f }, sheenRoughnessFactor{ 0.f },
				// KHR_materials_specular
				specularFactor{ 1.f }, specularColorFactor{ 1.f },
				// KHR_materials_transmission
				transmissionFactor{ 0.f },
				// KHR_materials_volume
				thicknessFactor{ 0.f }, attenuationDistance{ INFINITY }, attenuationColor{ 1.f }
			{}
#endif
			// Alpha
			uint32_t alphaMode;
			float alphaCutoff;

			uint32_t baseColorTexture;
			ALIGN_AS(16) VECTOR4 baseColorFactor;

			uint32_t metallicRoughnessTexture;
			float metallicFactor;
			float roughnessFactor;

			uint32_t normalTexture;
			float normalScale;

			uint32_t occlusionTexture;
			float occlusionStrenth;

			uint32_t emissiveTexture;
			ALIGN_AS(16) VECTOR3 emissiveFactor;

			uint32_t doubleSided;

			// KHR_materials_clearcoat
			float clearcoatFactor;
			float clearcoatRoughnessFactor;

			// KHR_materials_emissive_strength
			float emissiveStrength;

			// KHR_materials_ior
			float ior;

			// KHR_materials_iridescence
			float iridescenceFactor;
			float iridescenceIor;
			float iridescenceThicknessMinimum;
			float iridescenceThicknessMaximum;

			// KHR_materials_sheen
			ALIGN_AS(16) VECTOR3 sheenColorFactor;
			float sheenRoughnessFactor;

			// KHR_materials_specular
			float specularFactor;
			ALIGN_AS(16) VECTOR3 specularColorFactor;

			// KHR_materials_transmission
			float transmissionFactor;

			// KHR_materials_unlit ?

			// KHR_materials_volume
			float thicknessFactor;
			float attenuationDistance;
			ALIGN_AS(16) VECTOR3 attenuationColor;
		};


#ifdef __cplusplus
		struct FGlobalUniformData
#else
		layout(std140, set = 0, binding = 0) uniform GlobalUniformFrameData
#endif
		{
			float dt;
			uint32_t directionalLightCount;
			uint32_t pointLightCount;
			uint32_t spotLightCount;
			uint32_t lightCount;
			uint32_t cullerCount;
			uint32_t cameraCount;
			uint32_t maxCommandsCount;
			uint32_t rawCommandsCount;
			uint32_t defaultCameraID;
			uint32_t frameNumber;
#ifdef __cplusplus
		};
#else
		} frameData;
#endif

		// Base buffers
		REGISTER_OBJECT_ARRAY_BUFFER(NodeBuffer, FSceneNode, nodes);
		REGISTER_OBJECT_ARRAY_BUFFER(MatrixBuffer, MATRIX4, matrices);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshBuffer, FMesh, meshes);
		REGISTER_OBJECT_ARRAY_BUFFER(MaterialBuffer, FMaterial, materials);

		// Light buffers
		REGISTER_OBJECT_ARRAY_BUFFER(Lights, FLightCommit, lights);

		// Shadow buffers
		REGISTER_OBJECT_ARRAY_BUFFER(CascadedShadows, FCascadedShadowCommit, shadows);
		REGISTER_OBJECT_ARRAY_BUFFER(OmniShadows, FOmniShadowCommit, shadows);
		REGISTER_OBJECT_ARRAY_BUFFER(SpotShadows, FSpotShadowCommit, shadows);

		// Vertex buffer specific data
		REGISTER_OBJECT_ARRAY_BUFFER(RawVertexBuffer, FRawVertex, vertices);
		REGISTER_OBJECT_ARRAY_BUFFER(RawIndexBuffer, Index, indices);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshletBuffer, FMeshlet, meshlets);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshletVertexRemapBuffer, uint32_t, vertices);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshletBoundsBuffer, FGeometryDrawBounds, bounds);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshletTriangleBuffer, FTriangle, triangles);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshLodBuffer, FMeshLod, lods);

		// GPU object representation
		REGISTER_OBJECT_ARRAY_BUFFER(DrawInfoBuffer, FGeometryBatch, infos);
		REGISTER_OBJECT_ARRAY_BUFFER(ClusterInfoBuffer, FClusterInfo, clusters);
		REGISTER_OBJECT_ARRAY_BUFFER(FrustumSidesBuffer, VECTOR4, sides);

		// Indirect data
		REGISTER_OBJECT_ARRAY_BUFFER(IndirectTaskCommands, FDrawMeshIndirectData, commands);
		REGISTER_OBJECT_ARRAY_BUFFER(IndirectIndexedComands, FDrawIndexedIndirectData, commands);
		REGISTER_OBJECT_ARRAY_BUFFER(IndirectCountBuffer, uint32_t, indices);

		// Other
		REGISTER_OBJECT_ARRAY_BUFFER(TextureBindingRemap, uint32_t, remap);
		REGISTER_OBJECT_ARRAY_BUFFER(CameraBuffer, FCamera, cameras);

#ifdef __cplusplus
		struct FGlobalBufferReference
#else
		layout(std140, set = 0, binding = 1) uniform GlobalBufferReferences
#endif
		{
			NodeBuffer nodeBuffer;
			MatrixBuffer matrixBuffer;
			MeshBuffer meshBuffer;

			MaterialBuffer materialBuffer;
		
			Lights lightsBuffer;
		
			CascadedShadows cascadedShadowBuffer;
			OmniShadows omniShadowBuffer;
			SpotShadows spotShadowBuffer;
		
			DrawInfoBuffer drawInfoBuffer;
			DrawInfoBuffer rawDrawInfoBuffer;

			ClusterInfoBuffer clusterBuffer;
			FrustumSidesBuffer frustumSidesBuffer;

			IndirectIndexedComands indirectCommandBuffer;
			IndirectCountBuffer indirectCountBuffer;
			IndirectTaskCommands indirectTaskCommandBuffer;

			TextureBindingRemap textureRemapBuffer;
			CameraBuffer cameraBuffer;
		};

#ifdef __cplusplus
		struct FGlobalPostProcessingData
#else
		layout(std140, set = 0, binding = 2) uniform GlobalPostProcessingData
#endif
		{
			// fxaa
			int enable_fxaa;
			float fxaa_quality;
			float fxaa_threshold;
			float fxaa_threshold_min;
			// fxaa
			
			// tonemap
			int enable_tonemap;
			float tm_gamma;
			float tm_exposure;
			float tm_white_point;
			int tm_adaptive;
			// tonemap

			// bloom
			int enable_bloom;
			float b_bloom_strength;
			float b_threshold;
			float b_filter_radius;
			// bloom

			// dof
			int enable_dof;
			int df_sample_count;
			float df_poly;
			float df_focus_point;
			float df_near_field;
			float df_near_transition;
			float df_far_field;
			float df_far_transition;
			// dof

			// chromatic aberration
			int enable_chromatic_aberration;
			float ca_distortion;
			int ca_iterations;
			// chromatic aberration

			// fog
			int enable_fog;
			ALIGN_AS(16) VECTOR3 fog_color;
			float fog_density;
			float fog_max;
			// fog

			// film grain
			int enable_film_grain;
			float fg_amount;
			// film grain

			// vignette
			int enable_vignette;
			float v_inner_radius;
			float v_outer_radius;
			float v_opacity;
			// vignette

			// per object motion blur
			int enable_motion_blur;
			float mb_scale;
			int mb_max_samples;
			float mb_max_velocity;
			float mb_threshold;
			// per object motion blur

			// ao blur
			int blur_range;
			// ao blur
#ifdef __cplusplus
		};
#else
		} postprocess;
#endif

#ifdef __cplusplus
	}
}
#endif


#endif


	// Vertex inputs
#ifdef VERTEX_SHADER
		layout(location = 0) in vec3 inPosition;
#if defined(COLOR_VEC3)
		layout(location = 1) in vec3 inColor;
#elif defined(COLOR_VEC4)
		layout(location = 1) in vec4 inColor;
#endif // HAS_COLOR
#ifdef HAS_TEXCOORD
		layout(location = 2) in vec2 inTexCoord;
#endif // HAS_TEXCOORD
#ifdef HAS_NORMAL
		layout(location = 3) in vec3 inNormal;
#endif // HAS_NORMAL
#if defined(TANGENT_VEC3)
		layout(location = 4) in vec3 inTangent;
#elif defined(TANGENT_VEC4)
		layout(location = 4) in vec4 inTangent;
#endif // HAS_TANGENT
#ifdef HAS_SKIN
		layout(location = 5) in vec4 inJoints;
		layout(location = 6) in vec4 inWeights;
#endif // HAS_SKIN
#endif // VERTEX_SHADER

	// Bindless texture
	layout(set = 0, binding = 3) uniform sampler2D globalTextures[];

	#define GET_SAMPLER_2D(texture_index, mip_level) globalTextures[textureRemapBuffer.remap[texture_index] + mip_level]

	vec4 sampleTexture2D(uint32_t texture_index, uint32_t mip_level, vec2 uv)
	{
		return texture(GET_SAMPLER_2D(texture_index, mip_level), uv);
	}

	vec4 sampleTexture2DGrad(uint32_t texture_index, uint32_t mip_level, vec2 uv, vec2 uv_dx, vec2 uv_dy)
	{
		return textureGrad(GET_SAMPLER_2D(texture_index, mip_level), uv, uv_dx, uv_dy);
	}

//#if defined(COMPUTE_SHADER)
//		layout(std140, set = 1, binding = 0) uniform ComputePassCommonData
//		{
//			vec2 dispatchGridDim; 
//		};
//#elif defined(VERTEX_SHADER) || defined(TESS_CONTROL_SHADER) || defined(TESS_EVAL_SHADER) || defined(GEOMETRY_SHADER) || defined(FRAGMENT_SHADER)
//		layout(std140, set = 1, binding = 0) uniform GraphicsPassCommonData
//		{
//			uint32_t viewportIndex;
//		};
//#endif

	mat4 getMatrixById(uint global_offset, uint local_offset) { return matrixBuffer.matrices[global_offset + local_offset]; }

	mat4 getModelMatrix(uint global_offset) { return getMatrixById(global_offset, model_matrix_offset); }
	mat4 getOldModelMatrix(uint global_offset) { return getMatrixById(global_offset, old_model_matrix_offset); }

	mat4 getViewMatrix(uint global_offset) { return getMatrixById(global_offset, view_matrix_offset); }
	mat4 getInvViewMatrix(uint global_offset) { return getMatrixById(global_offset, inv_view_matrix_offset); }
	mat4 getProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, projection_matrix_offset); }
	mat4 getInvProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, inv_projection_matrix_offset); }
	mat4 getViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, view_projection_matrix_offset); }
	mat4 getPrevViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, prev_view_projection_matrix_offset); }
	mat4 getInvViewProjectionMatrix(uint global_offset) { return getMatrixById(global_offset, inv_view_projection_matrix_offset); }
	mat4 getReprojectionMatrix(uint global_offset) { return getMatrixById(global_offset, reprojection_matrix_offset); }
	
#endif


#ifndef nvidia_fxaa_protector
#define nvidia_fxaa_protector

		/*============================================================================
        
        
                            NVIDIA FXAA 3.11 by TIMOTHY LOTTES
        
        
        ------------------------------------------------------------------------------
        COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
        ------------------------------------------------------------------------------
        TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
        *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
        OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
        MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
        OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
        CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
        LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
        OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
        THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
        DAMAGES.
        
        ------------------------------------------------------------------------------
                                   INTEGRATION CHECKLIST
        ------------------------------------------------------------------------------
        (1.)
        In the shader source, setup defines for the desired configuration.
        When providing multiple shaders (for different presets),
        simply setup the defines differently in multiple files.
        Example,
        
          #define FXAA_PC 1
          #define FXAA_HLSL_5 1
          #define FXAA_QUALITY__PRESET 12
        
        Or,
        
          #define FXAA_360 1
          
        Or,
        
          #define FXAA_PS3 1
          
        Etc.
        
        (2.)
        Then include this file,
        
          #include "Fxaa3_11.h"
        
        (3.)
        Then call the FXAA pixel shader from within your desired shader.
        Look at the FXAA Quality FxaaPixelShader() for docs on inputs.
        As for FXAA 3.11 all inputs for all shaders are the same 
        to enable easy porting between platforms.
        
          return FxaaPixelShader(...);
        
        (4.)
        Insure pass prior to FXAA outputs RGBL (see next section).
        Or use,
        
          #define FXAA_GREEN_AS_LUMA 1
        
        (5.)
        Setup engine to provide the following constants
        which are used in the FxaaPixelShader() inputs,
        
          FxaaFloat2 fxaaQualityRcpFrame,
          FxaaFloat4 fxaaConsoleRcpFrameOpt,
          FxaaFloat4 fxaaConsoleRcpFrameOpt2,
          FxaaFloat4 fxaaConsole360RcpFrameOpt2,
          FxaaFloat fxaaQualitySubpix,
          FxaaFloat fxaaQualityEdgeThreshold,
          FxaaFloat fxaaQualityEdgeThresholdMin,
          FxaaFloat fxaaConsoleEdgeSharpness,
          FxaaFloat fxaaConsoleEdgeThreshold,
          FxaaFloat fxaaConsoleEdgeThresholdMin,
          FxaaFloat4 fxaaConsole360ConstDir
        
        Look at the FXAA Quality FxaaPixelShader() for docs on inputs.
        
        (6.)
        Have FXAA vertex shader run as a full screen triangle,
        and output "pos" and "fxaaConsolePosPos" 
        such that inputs in the pixel shader provide,
        
          // {xy} = center of pixel
          FxaaFloat2 pos,
        
          // {xy__} = upper left of pixel
          // {__zw} = lower right of pixel
          FxaaFloat4 fxaaConsolePosPos,
        
        (7.)
        Insure the texture sampler(s) used by FXAA are set to bilinear filtering.
        
        
        ------------------------------------------------------------------------------
                            INTEGRATION - RGBL AND COLORSPACE
        ------------------------------------------------------------------------------
        FXAA3 requires RGBL as input unless the following is set, 
        
          #define FXAA_GREEN_AS_LUMA 1
        
        In which case the engine uses green in place of luma,
        and requires RGB input is in a non-linear colorspace.
        
        RGB should be LDR (low dynamic range).
        Specifically do FXAA after tonemapping.
        
        RGB data as returned by a texture fetch can be non-linear,
        or linear when FXAA_GREEN_AS_LUMA is not set.
        Note an "sRGB format" texture counts as linear,
        because the result of a texture fetch is linear data.
        Regular "RGBA8" textures in the sRGB colorspace are non-linear.
        
        If FXAA_GREEN_AS_LUMA is not set,
        luma must be stored in the alpha channel prior to running FXAA.
        This luma should be in a perceptual space (could be gamma 2.0).
        Example pass before FXAA where output is gamma 2.0 encoded,
        
          color.rgb = ToneMap(color.rgb); // linear color output
          color.rgb = sqrt(color.rgb);    // gamma 2.0 color output
          return color;
        
        To use FXAA,
        
          color.rgb = ToneMap(color.rgb);  // linear color output
          color.rgb = sqrt(color.rgb);     // gamma 2.0 color output
          color.a = dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114)); // compute luma
          return color;
        
        Another example where output is linear encoded,
        say for instance writing to an sRGB formated render target,
        where the render target does the conversion back to sRGB after blending,
        
          color.rgb = ToneMap(color.rgb); // linear color output
          return color;
        
        To use FXAA,
        
          color.rgb = ToneMap(color.rgb); // linear color output
          color.a = sqrt(dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114))); // compute luma
          return color;
        
        Getting luma correct is required for the algorithm to work correctly.
        
        
        ------------------------------------------------------------------------------
                                  BEING LINEARLY CORRECT?
        ------------------------------------------------------------------------------
        Applying FXAA to a framebuffer with linear RGB color will look worse.
        This is very counter intuitive, but happends to be true in this case.
        The reason is because dithering artifacts will be more visiable 
        in a linear colorspace.
        
        
        ------------------------------------------------------------------------------
                                     COMPLEX INTEGRATION
        ------------------------------------------------------------------------------
        Q. What if the engine is blending into RGB before wanting to run FXAA?
        
        A. In the last opaque pass prior to FXAA,
           have the pass write out luma into alpha.
           Then blend into RGB only.
           FXAA should be able to run ok
           assuming the blending pass did not any add aliasing.
           This should be the common case for particles and common blending passes.
        
        A. Or use FXAA_GREEN_AS_LUMA.
        
        ============================================================================*/
        #definde FXAA_GREEN_AS_LUMA 1
        #ifndef FXAA_GREEN_AS_LUMA
            //
            // For those using non-linear color,
            // and either not able to get luma in alpha, or not wanting to,
            // this enables FXAA to run using green as a proxy for luma.
            // So with this enabled, no need to pack luma in alpha.
            //
            // This will turn off AA on anything which lacks some amount of green.
            // Pure red and blue or combination of only R and B, will get no AA.
            //
            // Might want to lower the settings for both,
            //    fxaaConsoleEdgeThresholdMin
            //    fxaaQualityEdgeThresholdMin
            // In order to insure AA does not get turned off on colors 
            // which contain a minor amount of green.
            //
            // 1 = On.
            // 0 = Off.
            //
            #define FXAA_GREEN_AS_LUMA 0
        #endif
        /*--------------------------------------------------------------------------*/
        #ifndef FXAA_EARLY_EXIT
            //
            // Controls algorithm's early exit path.
            // On PS3 turning this ON adds 2 cycles to the shader.
            // On 360 turning this OFF adds 10ths of a millisecond to the shader.
            // Turning this off on console will result in a more blurry image.
            // So this defaults to on.
            //
            // 1 = On.
            // 0 = Off.
            //
            #define FXAA_EARLY_EXIT 1
        #endif
        /*--------------------------------------------------------------------------*/
        #ifndef FXAA_DISCARD
            //
            // Only valid for PC OpenGL currently.
            // Probably will not work when FXAA_GREEN_AS_LUMA = 1.
            //
            // 1 = Use discard on pixels which don't need AA.
            //     For APIs which enable concurrent TEX+ROP from same surface.
            // 0 = Return unchanged color on pixels which don't need AA.
            //
            #define FXAA_DISCARD 0
        #endif
        
        #define FXAA_QUALITY__PS 12
        #define FXAA_QUALITY__P0 1.0
        #define FXAA_QUALITY__P1 1.0
        #define FXAA_QUALITY__P2 1.0
        #define FXAA_QUALITY__P3 1.0
        #define FXAA_QUALITY__P4 1.0
        #define FXAA_QUALITY__P5 1.5
        #define FXAA_QUALITY__P6 2.0
        #define FXAA_QUALITY__P7 2.0
        #define FXAA_QUALITY__P8 2.0
        #define FXAA_QUALITY__P9 2.0
        #define FXAA_QUALITY__P10 4.0
        #define FXAA_QUALITY__P11 8.0
        
        #define FxaaBool bool
        #define FxaaDiscard discard
        #define FxaaFloat float
        #define FxaaFloat2 vec2
        #define FxaaFloat3 vec3
        #define FxaaFloat4 vec4
        #define FxaaHalf float
        #define FxaaHalf2 vec2
        #define FxaaHalf3 vec3
        #define FxaaHalf4 vec4
        #define FxaaInt2 ivec2
        #define FxaaSat(x) clamp(x, 0.0, 1.0)
        #define FxaaTex sampler2D
        
        #define FxaaTexTop(t, p) textureLod(t, p, 0.0)
        #define FxaaTexOff(t, p, o) textureLodOffset(t, p, 0.0, o)
        #if (FXAA_GATHER4_ALPHA == 1)
        	#define FxaaTexAlpha4(t, p) textureGather(t, p, 3)
        	#define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)
        	#define FxaaTexGreen4(t, p) textureGather(t, p, 1)
        	#define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)
        #endif
        
        
        #if (FXAA_GREEN_AS_LUMA == 0)
            FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.w; }
        #else
            FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }
        #endif
        
        FxaaFloat4 FxaaPixelShader(
            //
            // Use noperspective interpolation here (turn off perspective interpolation).
            // {xy} = center of pixel
            FxaaFloat2 pos,
            //
            // Input color texture.
            // {rgb_} = color in linear or perceptual color space
            // if (FXAA_GREEN_AS_LUMA == 0)
            //     {___a} = luma in perceptual color space (not linear)
            FxaaTex tex,
            //
            // Only used on FXAA Quality.
            // This used to be the FXAA_QUALITY__SUBPIX define.
            // It is here now to allow easier tuning.
            // Choose the amount of sub-pixel aliasing removal.
            // This can effect sharpness.
            //   1.00 - upper limit (softer)
            //   0.75 - default amount of filtering
            //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
            //   0.25 - almost off
            //   0.00 - completely off
            FxaaFloat fxaaQualitySubpix,
            //
            // Only used on FXAA Quality.
            // This used to be the FXAA_QUALITY__EDGE_THRESHOLD define.
            // It is here now to allow easier tuning.
            // The minimum amount of local contrast required to apply algorithm.
            //   0.333 - too little (faster)
            //   0.250 - low quality
            //   0.166 - default
            //   0.125 - high quality 
            //   0.063 - overkill (slower)
            FxaaFloat fxaaQualityEdgeThreshold,
            //
            // Only used on FXAA Quality.
            // This used to be the FXAA_QUALITY__EDGE_THRESHOLD_MIN define.
            // It is here now to allow easier tuning.
            // Trims the algorithm from processing darks.
            //   0.0833 - upper limit (default, the start of visible unfiltered edges)
            //   0.0625 - high quality (faster)
            //   0.0312 - visible limit (slower)
            // Special notes when using FXAA_GREEN_AS_LUMA,
            //   Likely want to set this to zero.
            //   As colors that are mostly not-green
            //   will appear very dark in the green channel!
            //   Tune by looking at mostly non-green content,
            //   then start at zero and increase until aliasing is a problem.
            FxaaFloat fxaaQualityEdgeThresholdMin
        ) {
        /*--------------------------------------------------------------------------*/
        	vec2 fxaaQualityRcpFrame = vec2(1.0) / textureSize(tex, 0);
        
        	bool disc = false;
            FxaaFloat2 posM;
            posM.x = pos.x;
            posM.y = pos.y;
            #if (FXAA_GATHER4_ALPHA == 1)
                #if (FXAA_DISCARD == 0)
                    FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
                    #if (FXAA_GREEN_AS_LUMA == 0)
                        #define lumaM rgbyM.w
                    #else
                        #define lumaM rgbyM.y
                    #endif
                #endif
                #if (FXAA_GREEN_AS_LUMA == 0)
                    FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);
                    FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));
                #else
                    FxaaFloat4 luma4A = FxaaTexGreen4(tex, posM);
                    FxaaFloat4 luma4B = FxaaTexOffGreen4(tex, posM, FxaaInt2(-1, -1));
                #endif
                #if (FXAA_DISCARD == 1)
                    #define lumaM luma4A.w
                #endif
                #define lumaE luma4A.z
                #define lumaS luma4A.x
                #define lumaSE luma4A.y
                #define lumaNW luma4B.w
                #define lumaN luma4B.z
                #define lumaW luma4B.x
            #else
                FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);
                #if (FXAA_GREEN_AS_LUMA == 0)
                    #define lumaM rgbyM.w
                #else
                    #define lumaM rgbyM.y
                #endif
                FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1)));
                FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0)));
                FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1)));
                FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0)));
            #endif
        /*--------------------------------------------------------------------------*/
            FxaaFloat maxSM = max(lumaS, lumaM);
            FxaaFloat minSM = min(lumaS, lumaM);
            FxaaFloat maxESM = max(lumaE, maxSM);
            FxaaFloat minESM = min(lumaE, minSM);
            FxaaFloat maxWN = max(lumaN, lumaW);
            FxaaFloat minWN = min(lumaN, lumaW);
            FxaaFloat rangeMax = max(maxWN, maxESM);
            FxaaFloat rangeMin = min(minWN, minESM);
            FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
            FxaaFloat range = rangeMax - rangeMin;
            FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
            FxaaBool earlyExit = range < rangeMaxClamped;
        /*--------------------------------------------------------------------------*/
            if(earlyExit)
                #if (FXAA_DISCARD == 1)
                    FxaaDiscard;
                #else
                    disc = true;
                #endif
        /*--------------------------------------------------------------------------*/
            #if (FXAA_GATHER4_ALPHA == 0)
                FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1)));
                FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1)));
                FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1)));
                FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1)));
            #else
                FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1)));
                FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1)));
            #endif
        /*--------------------------------------------------------------------------*/
            FxaaFloat lumaNS = lumaN + lumaS;
            FxaaFloat lumaWE = lumaW + lumaE;
            FxaaFloat subpixRcpRange = 1.0/range;
            FxaaFloat subpixNSWE = lumaNS + lumaWE;
            FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;
            FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;
        /*--------------------------------------------------------------------------*/
            FxaaFloat lumaNESE = lumaNE + lumaSE;
            FxaaFloat lumaNWNE = lumaNW + lumaNE;
            FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
            FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
        /*--------------------------------------------------------------------------*/
            FxaaFloat lumaNWSW = lumaNW + lumaSW;
            FxaaFloat lumaSWSE = lumaSW + lumaSE;
            FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
            FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
            FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
            FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
            FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;
            FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;
        /*--------------------------------------------------------------------------*/
            FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;
            FxaaFloat lengthSign = fxaaQualityRcpFrame.x;
            FxaaBool horzSpan = edgeHorz >= edgeVert;
            FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
        /*--------------------------------------------------------------------------*/
            if(!horzSpan) lumaN = lumaW;
            if(!horzSpan) lumaS = lumaE;
            if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;
            FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;
        /*--------------------------------------------------------------------------*/
            FxaaFloat gradientN = lumaN - lumaM;
            FxaaFloat gradientS = lumaS - lumaM;
            FxaaFloat lumaNN = lumaN + lumaM;
            FxaaFloat lumaSS = lumaS + lumaM;
            FxaaBool pairN = abs(gradientN) >= abs(gradientS);
            FxaaFloat gradient = max(abs(gradientN), abs(gradientS));
            if(pairN) lengthSign = -lengthSign;
            FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);
        /*--------------------------------------------------------------------------*/
            FxaaFloat2 posB;
            posB.x = posM.x;
            posB.y = posM.y;
            FxaaFloat2 offNP;
            offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
            offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;
            if(!horzSpan) posB.x += lengthSign * 0.5;
            if( horzSpan) posB.y += lengthSign * 0.5;
        /*--------------------------------------------------------------------------*/
            FxaaFloat2 posN;
            posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;
            posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;
            FxaaFloat2 posP;
            posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;
            posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;
            FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;
            FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));
            FxaaFloat subpixE = subpixC * subpixC;
            FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));
        /*--------------------------------------------------------------------------*/
            if(!pairN) lumaNN = lumaSS;
            FxaaFloat gradientScaled = gradient * 1.0/4.0;
            FxaaFloat lumaMM = lumaM - lumaNN * 0.5;
            FxaaFloat subpixF = subpixD * subpixE;
            FxaaBool lumaMLTZero = lumaMM < 0.0;
        /*--------------------------------------------------------------------------*/
            lumaEndN -= lumaNN * 0.5;
            lumaEndP -= lumaNN * 0.5;
            FxaaBool doneN = abs(lumaEndN) >= gradientScaled;
            FxaaBool doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;
            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;
            FxaaBool doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;
            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;
        /*--------------------------------------------------------------------------*/
            if(doneNP) {
                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;
                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;
                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;
        /*--------------------------------------------------------------------------*/
                #if (FXAA_QUALITY__PS > 3)
                if(doneNP) {
                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                    doneN = abs(lumaEndN) >= gradientScaled;
                    doneP = abs(lumaEndP) >= gradientScaled;
                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;
                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;
                    doneNP = (!doneN) || (!doneP);
                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;
                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;
        /*--------------------------------------------------------------------------*/
                    #if (FXAA_QUALITY__PS > 4)
                    if(doneNP) {
                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                        doneN = abs(lumaEndN) >= gradientScaled;
                        doneP = abs(lumaEndP) >= gradientScaled;
                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;
                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;
                        doneNP = (!doneN) || (!doneP);
                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;
                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;
        /*--------------------------------------------------------------------------*/
                        #if (FXAA_QUALITY__PS > 5)
                        if(doneNP) {
                            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                            doneN = abs(lumaEndN) >= gradientScaled;
                            doneP = abs(lumaEndP) >= gradientScaled;
                            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;
                            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;
                            doneNP = (!doneN) || (!doneP);
                            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;
                            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;
        /*--------------------------------------------------------------------------*/
                            #if (FXAA_QUALITY__PS > 6)
                            if(doneNP) {
                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                                doneN = abs(lumaEndN) >= gradientScaled;
                                doneP = abs(lumaEndP) >= gradientScaled;
                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;
                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;
                                doneNP = (!doneN) || (!doneP);
                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;
                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;
        /*--------------------------------------------------------------------------*/
                                #if (FXAA_QUALITY__PS > 7)
                                if(doneNP) {
                                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                                    doneN = abs(lumaEndN) >= gradientScaled;
                                    doneP = abs(lumaEndP) >= gradientScaled;
                                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;
                                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;
                                    doneNP = (!doneN) || (!doneP);
                                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;
                                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;
        /*--------------------------------------------------------------------------*/
            #if (FXAA_QUALITY__PS > 8)
            if(doneNP) {
                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;
                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;
                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;
        /*--------------------------------------------------------------------------*/
                #if (FXAA_QUALITY__PS > 9)
                if(doneNP) {
                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                    doneN = abs(lumaEndN) >= gradientScaled;
                    doneP = abs(lumaEndP) >= gradientScaled;
                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;
                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;
                    doneNP = (!doneN) || (!doneP);
                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;
                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;
        /*--------------------------------------------------------------------------*/
                    #if (FXAA_QUALITY__PS > 10)
                    if(doneNP) {
                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                        doneN = abs(lumaEndN) >= gradientScaled;
                        doneP = abs(lumaEndP) >= gradientScaled;
                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;
                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;
                        doneNP = (!doneN) || (!doneP);
                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;
                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;
        /*--------------------------------------------------------------------------*/
                        #if (FXAA_QUALITY__PS > 11)
                        if(doneNP) {
                            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                            doneN = abs(lumaEndN) >= gradientScaled;
                            doneP = abs(lumaEndP) >= gradientScaled;
                            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;
                            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;
                            doneNP = (!doneN) || (!doneP);
                            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;
                            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;
        /*--------------------------------------------------------------------------*/
                            #if (FXAA_QUALITY__PS > 12)
                            if(doneNP) {
                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));
                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));
                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                                doneN = abs(lumaEndN) >= gradientScaled;
                                doneP = abs(lumaEndP) >= gradientScaled;
                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;
                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;
                                doneNP = (!doneN) || (!doneP);
                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;
                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;
        /*--------------------------------------------------------------------------*/
                            }
                            #endif
        /*--------------------------------------------------------------------------*/
                        }
                        #endif
        /*--------------------------------------------------------------------------*/
                    }
                    #endif
        /*--------------------------------------------------------------------------*/
                }
                #endif
        /*--------------------------------------------------------------------------*/
            }
            #endif
        /*--------------------------------------------------------------------------*/
                                }
                                #endif
        /*--------------------------------------------------------------------------*/
                            }
                            #endif
        /*--------------------------------------------------------------------------*/
                        }
                        #endif
        /*--------------------------------------------------------------------------*/
                    }
                    #endif
        /*--------------------------------------------------------------------------*/
                }
                #endif
        /*--------------------------------------------------------------------------*/
            }
        /*--------------------------------------------------------------------------*/
            FxaaFloat dstN = posM.x - posN.x;
            FxaaFloat dstP = posP.x - posM.x;
            if(!horzSpan) dstN = posM.y - posN.y;
            if(!horzSpan) dstP = posP.y - posM.y;
        /*--------------------------------------------------------------------------*/
            FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
            FxaaFloat spanLength = (dstP + dstN);
            FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
            FxaaFloat spanLengthRcp = 1.0/spanLength;
        /*--------------------------------------------------------------------------*/
            FxaaBool directionN = dstN < dstP;
            FxaaFloat dst = min(dstN, dstP);
            FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;
            FxaaFloat subpixG = subpixF * subpixF;
            FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
            FxaaFloat subpixH = subpixG * fxaaQualitySubpix;
        /*--------------------------------------------------------------------------*/
            FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
            FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
            if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
            if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
            #if (FXAA_DISCARD == 1)
                return FxaaTexTop(tex, posM);
            #else
                return disc ? rgbyM : FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);
            #endif
        }
	
#endif


#ifndef util_protector
#define util_protector

		vec4 sRGBToLinear(in vec4 srgbIn)
        {
            vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
        	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
        	return vec4(linOut,srgbIn.w);
        }
        
        vec4 linearTosRGB(vec4 linearRGB)
        {
            bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
            vec4 higher = vec4(1.055) * pow(linearRGB, vec4(1.0 / 2.4)) - vec4(0.055);
            vec4 lower = linearRGB * vec4(12.92);
        
            return mix(higher, lower, cutoff);
        }
        
        // From http://filmicworlds.com/blog/filmic-tonemapping-operators/
        vec3 Uncharted2Tonemap(vec3 color)
        {
        	float A = 0.15f;
        	float B = 0.50f;
        	float C = 0.10f;
        	float D = 0.20f;
        	float E = 0.02f;
        	float F = 0.30f;
        	float W = 11.2f;
        	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
        }
        
        vec3 getTangentSpaceNormalMap(in vec3 normalColor, mat3 TBN, float scale)
        {
            vec3 normal = normalize(TBN * ((2.0 * normalColor - 1.0) * vec3(scale, scale, 1.0)));
        	return normal;
        }
        
        // Calculate normal in tangent space
        mat3 calculateTBN(vec3 normal, vec4 tangent)
        {
        	vec3 N = normalize(normal);
        	vec3 T = normalize(tangent.xyz);
            vec3 B = normalize(cross(N, T) * tangent.w);
        	
        	return mat3(T, B, N);
        }

		// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
		float random(vec2 co)
		{
			float a = 12.9898f;
			float b = 78.233f;
			float c = 43758.5453f;
			float dt= dot(co.xy ,vec2(a,b));
			float sn= mod(dt,3.14f);
			return fract(sin(sn) * c);
		}

		vec3 getPositionFromDepth(vec2 texcoord, float depth, mat4 invViewProjection)
        {
            vec4 clipSpaceLocation;
            clipSpaceLocation.x = texcoord.x * 2.0f - 1.0f;
            clipSpaceLocation.y = texcoord.y * 2.0f - 1.0f;
            clipSpaceLocation.z = depth;
            clipSpaceLocation.w = 1.0f;
            vec4 homogenousLocation = invViewProjection * clipSpaceLocation;
            return homogenousLocation.xyz / homogenousLocation.w;
        }

        #ifdef FRAGMENT_SHADER
        vec3 getNormalFromDepth(in sampler2D depth_sampler, in vec2 uv, in mat4 invProjection)
        {
            float depth = texture(depth_sampler, uv).x;
        
            vec4 clipSpace = vec4(uv * 2.0 - 1.0, depth, 1.0);
            vec4 viewSpace = invProjection * clipSpace;
            viewSpace.xyz /= viewSpace.w;
        
            vec3 pos = viewSpace.xyz;
            vec3 n = normalize(cross(dFdx(pos), dFdy(pos)));
            n *= -1;
        
            return n;
        }
        #endif

        float linearizeDepth(in float depth, in float zNear, in float zFar)
        {
            float z = depth * 2.0f - 1.0f;
            return (2.0f * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
        }
        
        uvec4 packTextures(vec3 normal_map, vec3 albedo_map, vec4 pbr_map)
        {
            uvec4 texture_pack;
            //Storing RG components of normal map in 32bit R component
            texture_pack.r = packHalf2x16(normal_map.rg);
            //Storing B component of normal map and R component of albedo in G
            texture_pack.g = packHalf2x16(vec2(normal_map.b, albedo_map.r));
            //Storing GB components of albedo in B
            texture_pack.b = packHalf2x16(albedo_map.gb);
            // Store metal, roughness, ao and heignt in A component
            texture_pack.a = packUnorm4x8(pbr_map);
            return texture_pack;
        }
        
        void unpackTextures(in uvec4 texture_pack, out vec3 normal_map, out vec3 albedo_map, out vec4 pbr_map)
        {
            vec2 pack_R = unpackHalf2x16(texture_pack.r);
        	vec2 pack_G = unpackHalf2x16(texture_pack.g);
        	vec2 pack_B = unpackHalf2x16(texture_pack.b);
        	pbr_map = unpackUnorm4x8(texture_pack.a);
            normal_map = vec3(pack_R, pack_G.r);
            albedo_map = vec3(pack_G.g, pack_B);
        }
        
        vec4 per_object_motion_blir(sampler2D velocity_texture, sampler2D color_texture, vec2 inUV, float velocityScale, int max_samples)
        {
            vec2 texelSize = 1.0 / vec2(textureSize(color_texture, 0));
            vec2 velocity = texture(velocity_texture, inUV * vec2(1.0, -1.0)).rg * velocityScale;
            float speed = length(velocity / texelSize);
            int samples = clamp(int(speed), 1, max_samples);
        
            vec4 motionBlur = texture(color_texture, inUV);
            for (int i = 1; i < samples; ++i) 
            {
                vec2 offset = velocity * (float(i) / float(samples - 1) - 0.5);
                motionBlur += texture(color_texture, inUV + offset);
            }
            motionBlur /= float(samples);
            return motionBlur;
        }

        vec2 encodeVelocity(in vec2 velocity)
        {
#ifdef LOW_PRECISION_VELOCITY
            return pow(velocity * 0.5 + 0.5, vec2(3.0));
#else
            return velocity * 0.5 + 0.5;
#endif
        }
        
        vec2 decodeVelocity(in vec2 velocity)
        {
#ifdef LOW_PRECISION_VELOCITY
            return pow(velocity, vec2(1.0 / 3.0)) * 2.0 - 1.0;
#else
            return velocity * 2.0 - 1.0;
#endif
        }


        vec2 barrelDistortion(vec2 coord, float amt) {
			vec2 cc = coord - 0.5;
			float dist = dot(cc, cc);
			return coord + cc * dist * amt;
		}

        float saturate(float t) { return clamp(t, 0.0, 1.0); }
        vec2 saturate(vec2 t) { return clamp(t, vec2(0.0), vec2(1.0)); }
        vec3 saturate(vec3 t) { return clamp(t, vec3(0.0), vec3(1.0)); }
        vec4 saturate(vec4 t) { return clamp(t, vec4(0.0), vec4(1.0)); }

        bool is_saturated(float a) { return a == saturate(a); }
        bool is_saturated(vec2 a) { return a == saturate(a); }
        bool is_saturated(vec3 a) { return a == saturate(a); }
        bool is_saturated(vec4 a) { return a == saturate(a); }
		
		float linterp( float t ) {
			return saturate( 1.0 - abs( 2.0*t - 1.0 ) );
		}
		
		float remap( float t, float a, float b ) {
			return saturate( (t - a) / (b - a) );
		}
		
		vec4 spectrum_offset( float t ) {
			vec4 ret;
			float lo = step(t,0.5);
			float hi = 1.0-lo;
			float w = linterp( remap( t, 1.0/6.0, 5.0/6.0 ) );
			ret = vec4(lo,1.0,hi, 1.) * vec4(1.0-w, w, 1.0-w, 1.);
		
			return pow( ret, vec4(1.0/2.2) );
		}

		vec3 convertXYZ2Yxy(vec3 _xyz)
		{
			// Reference(s):
			// - XYZ to xyY
			//   https://web.archive.org/web/20191027010144/http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
			float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
			return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
		}
		
		vec3 convertXYZ2RGB(vec3 _xyz)
		{
			vec3 rgb;
			rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
			rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
			rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
			return rgb;
		}
		
		vec3 convertRGB2XYZ(vec3 _rgb)
		{
			// Reference(s):
			// - RGB/XYZ Matrices
			//   https://web.archive.org/web/20191027010220/http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
			vec3 xyz;
			xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
			xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
			xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
			return xyz;
		}
		
		vec3 convertYxy2XYZ(vec3 _Yxy)
		{
			// Reference(s):
			// - xyY to XYZ
			//   https://web.archive.org/web/20191027010036/http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
			vec3 xyz;
			xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
			xyz.y = _Yxy.x;
			xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
			return xyz;
		}
		
		vec3 convertRGB2Yxy(vec3 _rgb)
		{
			return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
		}
		
		vec3 convertYxy2RGB(vec3 _Yxy)
		{
			return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
		}
		
		float reinhard2(float _x, float _whiteSqr)
		{
			return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
		}
		
		vec3 toGamma(vec3 _rgb, float gamma)
		{
			return pow(abs(_rgb), vec3(1.0 / gamma) );
		}
	
#endif


		layout(local_size_x = 8, local_size_y = 8) in;

		layout(set = 1, binding = 0) uniform writeonly image2D write_color;
		layout(set = 1, binding = 1) uniform sampler2D sampler_color;

#if defined(MOTION_BLUR)
		layout(set = 1, binding = 2) uniform sampler2D motion_vectors;
#elif defined(BLOOM)
		layout(set = 1, binding = 2) uniform sampler2D bloom_color;
		layout(set = 1, binding = 3) uniform sampler2D dirt_mask;
#elif defined(DOF)
		layout(set = 1, binding = 2) uniform sampler2D bokeh_color;
		layout(set = 1, binding = 3) uniform sampler2D depth_image;
#elif defined(TONEMAP)
		layout(set = 1, binding = 2, r32f) uniform image2D avg_luminosity;
#endif

#if defined(GAUSSIAN_BLUR)
		layout(push_constant) uniform FConstantBlock
		{
			float blur_scale;
		    float blur_strength;
		} constants;
#endif
		
		void main()
		{
			uvec2 image_size = imageSize(write_color);
		
			uvec2 px = uvec2(0, 0);
			vec2 uv = vec2(0.0, 0.0);
			if(swizzle_workgroups_8x8(gl_GlobalInvocationID.xy, gl_LocalInvocationID.xy, gl_WorkGroupID.xy, image_size, uv, px))
				return;

			FCamera camera = cameraBuffer.cameras[frameData.defaultCameraID];
			
			vec4 color = texture(sampler_color, uv);
			vec2 texel_size = 1.0 / vec2(textureSize(sampler_color, 0));
#if defined(FXAA)
			if(postprocess.enable_fxaa > 0)
			{
				fragcolor = FxaaPixelShader(uv, sampler_color, postprocess.fxaa_quality, postprocess.fxaa_threshold, postprocess.fxaa_threshold_min).rgb;
			}
#elif defined(MOTION_BLUR)
			if(postprocess.enable_motion_blur > 0)
			{
				vec2 velocity = texture(motion_vectors, uv).rg;
				velocity = decodeVelocity(velocity) * postprocess.mb_scale;
				velocity = min(vec2(1.0), velocity);

				if(length(velocity) >= postprocess.mb_threshold)
				{
					float speed = length(velocity / texel_size);
					int n_samples = clamp(int(speed), 1, postprocess.mb_max_samples);

					for (int i = 1; i < n_samples; ++i) 
					{
						vec2 offset = velocity * (float(i) / float(n_samples - 1) - 0.5);
						color += texture(sampler_color, uv + offset);
					}
					color /= float(n_samples);
				}
			}
#elif defined(BLOOM)
			if(postprocess.enable_bloom > 0)
			{
				const float dirtMaskIntensity = 1.0f;
				vec3 dirt = texture(dirt_mask, vec2(uv.x, 1.f - uv.y)).rgb * dirtMaskIntensity;
				vec3 bloom = texture(bloom_color, uv).rgb;
				color.rgb = mix(color.rgb, bloom + bloom * dirt, vec3(postprocess.b_bloom_strength));
			}
#elif defined(DOF)
			if(postprocess.enable_dof > 0)
			{
				float depth = texture(depth_image, uv).r;
				float linear_depth = linearizeDepth(depth, camera.zNear, camera.zFar);

				vec3 texture_blured = texture(bokeh_color, uv).rgb;

				float near_end = postprocess.df_near_field + postprocess.df_near_transition;
				float far_start = postprocess.df_far_field - postprocess.df_far_transition;
		
				float near_visibility = smoothstep(postprocess.df_near_field * postprocess.df_focus_point, near_end * postprocess.df_focus_point, linear_depth);
				float far_visibility = 1.0f - smoothstep(far_start * postprocess.df_focus_point, postprocess.df_far_field * postprocess.df_focus_point, linear_depth);
		
				vec3 temp = mix(texture_blured, color.rgb, near_visibility);
				color.rgb = mix(texture_blured, temp, far_visibility);
			}
#elif defined(CHROMATIC_ABERRATION)
			if(postprocess.enable_chromatic_aberration > 0)
			{
				const float reci_num_iter_f = 1.0 / float(postprocess.ca_iterations);
		
				vec4 sumcol = vec4(0.0);
				vec4 sumw = vec4(0.0);	
				for ( int i= 0; i < postprocess.ca_iterations; ++i )
				{
					float t = float(i) * reci_num_iter_f;
					vec4 w = spectrum_offset( t );
					sumw += w;
					sumcol += w * texture( sampler_color, barrelDistortion(uv, 0.6 * postprocess.ca_distortion * t ) );
				}

				color.rgb = (sumcol / sumw).rgb;
			}
#elif defined(TONEMAP)
			if(postprocess.enable_tonemap > 0)
			{
				float avgLum = imageLoad(avg_luminosity, ivec2(0, 0)).x;
		
				vec3 Yxy = convertRGB2Yxy(color.rgb);
		
				float lp = Yxy.x / (9.6 * avgLum + 0.0001);
		
				if(postprocess.tm_adaptive > 0)
					Yxy.x = reinhard2(lp, postprocess.tm_white_point);
				else
					Yxy.x = Yxy.x * pow(2.0, postprocess.tm_exposure);
		
				color.rgb = convertYxy2RGB(Yxy);
				color.rgb = toGamma(color.rgb, postprocess.tm_gamma);
			}
#elif defined(GAUSSIAN_BLUR)
#ifdef HORIZONTAL
				vec2 direction = vec2(constants.blur_strength, 0.0);
#else
				vec2 direction = vec2(0.0, constants.blur_strength);
#endif // HORIZONTAL

#if defined(BLUR_TYPE_5)
			{
				vec2 off1 = vec2(1.3333333333333333f) * direction;
				color += texture(sampler_color, uv) * 0.29411764705882354 * constants.blur_scale;
				color += texture(sampler_color, uv + (off1 / image_size)) * 0.35294117647058826f * constants.blur_scale;
				color += texture(sampler_color, uv - (off1 / image_size)) * 0.35294117647058826f * constants.blur_scale;
			}
#elif defined(BLUR_TYPE_9)
			{
				vec2 off1 = vec2(1.3846153846f) * direction;
				vec2 off2 = vec2(3.2307692308f) * direction;
				color += texture(sampler_color, uv) * 0.2270270270f * constants.blur_scale;
				color += texture(sampler_color, uv + (off1 / image_size)) * 0.3162162162f * constants.blur_scale;
				color += texture(sampler_color, uv - (off1 / image_size)) * 0.3162162162f * constants.blur_scale;
				color += texture(sampler_color, uv + (off2 / image_size)) * 0.0702702703f * constants.blur_scale;
				color += texture(sampler_color, uv - (off2 / image_size)) * 0.0702702703f * constants.blur_scale;
			}
#elif defined(BLUR_TYPE_13)
			{
				vec2 off1 = vec2(1.411764705882353f) * direction;
				vec2 off2 = vec2(3.2941176470588234f) * direction;
				vec2 off3 = vec2(5.176470588235294f) * direction;
				color += texture(sampler_color, uv) * 0.1964825501511404f * constants.blur_scale;
				color += texture(sampler_color, uv + (off1 / image_size)) * 0.2969069646728344f * constants.blur_scale;
				color += texture(sampler_color, uv - (off1 / image_size)) * 0.2969069646728344f * constants.blur_scale;
				color += texture(sampler_color, uv + (off2 / image_size)) * 0.09447039785044732f * constants.blur_scale;
				color += texture(sampler_color, uv - (off2 / image_size)) * 0.09447039785044732f * constants.blur_scale;
				color += texture(sampler_color, uv + (off3 / image_size)) * 0.010381362401148057f * constants.blur_scale;
				color += texture(sampler_color, uv - (off3 / image_size)) * 0.010381362401148057f * constants.blur_scale;
			}
#endif // BLUR_TYPE
#elif defined(LENSFLARE)

			//vec2 main = uv-pos;
			//vec2 uvd = uv*(length(uv));
			//
			//float ang = atan(main.x,main.y);
			//float dist=length(main); dist = pow(dist,.1);
			//float n = noise(vec2(ang*16.0,dist*32.0));
			//
			//float f0 = 1.0/(length(uv-pos)*16.0+1.0);
			//
			//f0 = f0 + f0*(sin(noise(sin(ang*2.+pos.x)*4.0 - cos(ang*3.+pos.y))*16.)*.1 + dist*.1 + .8);
			//
			//float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*7.0;
			//
			//float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
			//float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
			//float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;
			//
			//vec2 uvx = mix(uv,uvd,-0.5);
			//
			//float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
			//float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
			//float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;
			//
			//uvx = mix(uv,uvd,-.4);
			//
			//float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
			//float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
			//float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;
			//
			//uvx = mix(uv,uvd,-0.5);
			//
			//float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
			//float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
			//float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;
			//
			//vec3 c = vec3(.0);
			//
			//c.r+=f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
			//c = c*1.3 - vec3(length(uvd)*.05);
			//c+=vec3(f0);
			//
			//color.rgb *= c;
#endif
		
			imageStore(write_color, ivec2(px), color);
		}
	
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
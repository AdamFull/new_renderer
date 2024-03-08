#ifndef INTEROP_BASE_H
#define INTEROP_BASE_H

#ifdef __cplusplus

#include <glm/glm.hpp>
#include <standart_library.h>

#define ALIGN_AS(x) alignas(x)

#define ARRAY_OF(name, type, count) stl::array<type, count> name

#define CONSTANT constexpr const

#define VECTOR2 glm::vec2
#define VECTOR3 glm::vec3
#define VECTOR4 glm::vec4
#define MATRIX2 glm::mat2
#define MATRIX3 glm::mat3
#define MATRIX4 glm::mat4

#define REGISTER_OBJECT_ARRAY_BUFFER(BufferTypeName, DataType, ArrayName) typedef uint64_t BufferTypeName
#else // __cplusplus
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
		CONSTANT uint32_t max_point_light_shadow_count = 1u;
		CONSTANT uint32_t point_light_shadow_map_resolution = 512u;

#if defined(INDEX_8BIT)
		typedef uint8_t Index;
#elif defined(INDEX_16BIT)
		typedef uint16_t Index;
#else
		typedef uint32_t Index;
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

		struct FGeometryGPUCullInfo
		{
			uint32_t instanceID;
			uint32_t frustumID;
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
			uint32_t meshletOffset;
			uint32_t meshletVertexOffset;
			uint32_t meshletTriangleOffset;
			uint32_t meshletBoundsOffset;
			uint32_t meshletCount;
		}; // For lod buffer

		struct FMesh
		{
			uint32_t materialID;

			uint32_t lodOffset;
			uint32_t lodCount;
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
			MATRIX4 model;
			uint32_t skeletonID;
			uint32_t pad[3];
		};

		struct FDrawMeshinfo
		{
			uint32_t vboID;
			uint32_t meshID;
			uint32_t nodeID;
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

		// Lights and shadows
		struct FCascadedShadowCommit
		{
			ARRAY_OF(splitDepths, float, cascade_shadow_map_cascade_count);
			ARRAY_OF(cascadeViewProj, MATRIX4, cascade_shadow_map_cascade_count);
			float farClip;
			uint32_t pad;
		};

		struct FDirectionalLightCommit
		{
			ALIGN_AS(16) VECTOR3 direction;
			ALIGN_AS(16) VECTOR3 color;
			float intencity;
			int shadowIndex;
			int castShadows;
			uint32_t pad;
		};

		struct FOmniShadowCommit
		{
			ARRAY_OF(viewProj, MATRIX4, 6u);
			float farPlane;
			int index;
		};

		struct FPointLightCommit
		{
			ALIGN_AS(16) VECTOR3 position;
			ALIGN_AS(16) VECTOR3 color;
			float intencity;
			float radius;
			int shadowIndex;
			int castShadows;
		};

		struct FSpotShadowCommit
		{
			MATRIX4 viewProj;
			int index;
			uint32_t pad;
		};

		struct FSpotLightCommit
		{
			ALIGN_AS(16) VECTOR3 position;
			ALIGN_AS(16) VECTOR3 direction;
			ALIGN_AS(16) VECTOR3 color;
			float intencity;
			float lightAngleScale;
			float lightAngleOffset;
			int toTarget;
			int shadowIndex;
			int castShadows;
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

		struct FGlobalUniformData
		{
			MATRIX4 view;
			MATRIX4 invView;
			MATRIX4 projection;
			MATRIX4 invProjection;
			MATRIX4 viewProjection;
			MATRIX4 invViewProjection;
			VECTOR2 viewportDimension;
			VECTOR2 invViewportDimension;
			float nearPlane;
			float farPlane;
			uint32_t directionalLightCount;
			uint32_t pointLightCount;
			uint32_t spotLightCount;
			uint32_t rawCommandsCount;
		};

#ifndef __cplusplus
#ifdef __glslang
		layout(std140, set = 0, binding = 0) uniform GlobalUniformFrameData { FGlobalUniformData frameData; };
#else // __glslang
		[[vk::binding(0, 0)]]
		cbuffer GlobalBufferReferences : register(b0) { FGlobalUniformData frameData; };
#endif // __glslang
#endif // __cplusplus

		// Base buffers
		REGISTER_OBJECT_ARRAY_BUFFER(NodeBuffer, FSceneNode, nodes);
		REGISTER_OBJECT_ARRAY_BUFFER(SkinBuffer, MATRIX4, matrices);
		REGISTER_OBJECT_ARRAY_BUFFER(MeshBuffer, FMesh, meshes);
		REGISTER_OBJECT_ARRAY_BUFFER(MaterialBuffer, FMaterial, materials);

		// Light buffers
		REGISTER_OBJECT_ARRAY_BUFFER(DirectionalLights, FDirectionalLightCommit, lights);
		REGISTER_OBJECT_ARRAY_BUFFER(PointLights, FPointLightCommit, lights);
		REGISTER_OBJECT_ARRAY_BUFFER(SpotLights, FSpotLightCommit, lights);

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
		REGISTER_OBJECT_ARRAY_BUFFER(DrawInfoBuffer, FDrawMeshinfo, infos);
		REGISTER_OBJECT_ARRAY_BUFFER(ClusterInfoBuffer, FClusterInfo, clusters);

		// Indirect data
		REGISTER_OBJECT_ARRAY_BUFFER(IndirectTaskCommands, FDrawMeshIndirectData, commands);
		REGISTER_OBJECT_ARRAY_BUFFER(IndirectIndexedComands, FDrawIndexedIndirectData, commands);

		// Other
		REGISTER_OBJECT_ARRAY_BUFFER(TextureBindingRemap, uint32_t, remap);

#ifdef __cplusplus
		struct FGlobalBufferReference
#elif defined(__glslang)
		layout(std140, set = 0, binding = 1) uniform GlobalBufferReferences
#else
		[[vk::binding(0, 1)]]
		cbuffer GlobalBufferReferences : register(b1)
#endif
		{
			NodeBuffer nodeBuffer;
			SkinBuffer jointsBuffer;
			MeshBuffer meshBuffer;

			MaterialBuffer materialBuffer;
		
			DirectionalLights directionalLightBuffer;
			PointLights pointLightBuffer;
			SpotLights spotLightBuffer;
		
			CascadedShadows cascadedShadowBuffer;
			OmniShadows omniShadowBuffer;
			SpotShadows spotShadowBuffer;
		
			DrawInfoBuffer drawInfoBuffer;
			ClusterInfoBuffer clusterBuffer;

			IndirectIndexedComands indirectCommandBuffer;
			IndirectTaskCommands indirectTaskCommandBuffer;

			TextureBindingRemap textureRemapBuffer;
		};

#ifdef __cplusplus
	}
}
#endif

#endif
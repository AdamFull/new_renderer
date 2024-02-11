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
#else
#define ALIGN_AS(x)

#define CONSTANT const

#define ARRAY_OF(name, type, count) type name[count]

#define VECTOR2 vec2
#define VECTOR3 vec3
#define VECTOR4 vec4
#define MATRIX2 mat2
#define MATRIX3 mat3
#define MATRIX4 mat4
#endif

#ifdef __cplusplus
namespace engine { 
	namespace render {
#endif
		CONSTANT uint32_t max_indirect_draw_count = 512u;

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

		struct FGeometryGPUCullInfo
		{
			uint32_t instanceID;
			uint32_t frustumID;
		};

		struct FGeometryDrawBounds
		{
			VECTOR3 min;
			uint32_t pad0;
			VECTOR3 max;
			uint32_t pad1;
		};

		// Verticea range
		struct FGeometryDrawRange
		{
			uint32_t beginIndex;
			uint32_t indexCount;
			uint32_t beginVertex;
			uint32_t vertexCount;

			FGeometryDrawBounds bounds;
		};

		struct FGeometryGPUBatch
		{
			uint32_t vboID;
			uint32_t materialID;
			uint32_t instanceID;
			uint32_t pad;
		};

		struct FDrawIndirectData
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
			uint32_t pad[3];

			FGeometryDrawBounds aabb;
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

		struct FSceneNode
		{
#ifdef __cplusplus
			FSceneNode() : model{ 1.f }, skin{ ~0u }, instances{ ~0u }, instanceCount{ 0u } {}
#endif
			MATRIX4 model;
			uint32_t skin;
			uint32_t instances;
			uint32_t instanceCount;
			uint32_t pad;
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
#ifdef __cplusplus
		};
#else
		} frameData;
#endif

// Global bindless buffers
#ifdef __cplusplus
		typedef uint64_t NodeStorage;
		typedef uint64_t MaterialStorage;
		typedef uint64_t SkinningJoints;
		typedef uint64_t GeometryBatches;
		typedef uint64_t IndirectIndexedComands;
		typedef uint64_t DirectionalLights;
		typedef uint64_t PointLights;
		typedef uint64_t SpotLights;
		typedef uint64_t CascadedShadows;
		typedef uint64_t OmniShadows;
		typedef uint64_t SpotShadows;
		typedef uint64_t TextureBindingRemap;
#else
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer NodeStorage { FSceneNode nodes[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer MaterialStorage { FMaterial materials[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer SkinningJoints { mat4 matrices[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) buffer GeometryBatches { FGeometryGPUBatch geometry_batches[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) buffer IndirectIndexedComands { FDrawIndexedIndirectData indexed_draw_commands[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer DirectionalLights { FDirectionalLightCommit directional_lights[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer PointLights { FPointLightCommit point_lights[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer SpotLights { FSpotLightCommit spot_lights[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer CascadedShadows { FCascadedShadowCommit cascaded_shadows[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer OmniShadows { FOmniShadowCommit omni_shadows[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer SpotShadows { FSpotShadowCommit spot_shadows[]; };
		layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer TextureBindingRemap { uint32_t texture_remap[]; };
#endif

#ifdef __cplusplus
		struct FGlobalBufferReference
#else
		layout(std140, set = 0, binding = 1) uniform GlobalBufferReferences
#endif
		{
			NodeStorage nodeBuffer;
			MaterialStorage materialBuffer;
			SkinningJoints jointsBuffer;
		
			DirectionalLights directionalLightBuffer;
			PointLights pointLightBuffer;
			SpotLights spotLightBuffer;
		
			CascadedShadows cascadedShadowBuffer;
			OmniShadows omniShadowBuffer;
			SpotShadows spotShadowBuffer;
		
			GeometryBatches batchBuffer;
			IndirectIndexedComands indirectCommandBuffer;
			TextureBindingRemap textureRemapBuffer;
		};

#ifdef __cplusplus
	}
}
#endif

#endif
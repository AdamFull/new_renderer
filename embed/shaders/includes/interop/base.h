#ifndef INTEROP_BASE_H
#define INTEROP_BASE_H

#ifdef __cplusplus

#include <glm/glm.hpp>
#include <array>

#define ALIGN_AS(x) alignas(x)

#define ARRAY_OF(name, type, count) std::array<type, count> name

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

// Directional light
CONSTANT uint32_t max_directional_light_count = 3u;
CONSTANT uint32_t max_directional_light_shadow_count = 1u;
CONSTANT uint32_t cascade_shadow_map_resolution = 2048u;
CONSTANT uint32_t cascade_shadow_map_cascade_count = 8u;

// Spot light
CONSTANT uint32_t max_spot_light_count = 64u;
CONSTANT uint32_t max_spot_light_shadow_count = 1u;
CONSTANT uint32_t spot_light_shadow_map_resolution = 1024u;

// Point light
CONSTANT uint32_t max_point_light_count = 64u;
CONSTANT uint32_t max_point_light_shadow_count = 1u;
CONSTANT uint32_t point_light_shadow_map_resolution = 512u;

// Lights and shadows
struct FCascadedShadowCommit
{
	ARRAY_OF(splitDepths, float, cascade_shadow_map_cascade_count);
	ARRAY_OF(cascadeViewProj, MATRIX4, cascade_shadow_map_cascade_count);
	ALIGN_AS(4) float farClip;
};

struct FDirectionalLightCommit
{
	ALIGN_AS(16) VECTOR3 direction;
	ALIGN_AS(16) VECTOR3 color;
	ALIGN_AS(4) float intencity;
	ALIGN_AS(4) int shadowIndex;
	ALIGN_AS(4) int castShadows;
};

struct FOmniShadowCommit
{
	ARRAY_OF(viewProj, MATRIX4, 6u);
	ALIGN_AS(4) float farPlane;
	ALIGN_AS(4) int index;
};

struct FPointLightCommit
{
	ALIGN_AS(16) VECTOR3 position;
	ALIGN_AS(16) VECTOR3 color;
	ALIGN_AS(4) float intencity;
	ALIGN_AS(4) float radius;
	ALIGN_AS(4) int shadowIndex;
	ALIGN_AS(4) int castShadows;
};

struct FSpotShadowCommit
{
	MATRIX4 viewProj;
	ALIGN_AS(4) int index;
};

struct FSpotLightCommit
{
	ALIGN_AS(16) VECTOR3 position;
	ALIGN_AS(16) VECTOR3 direction;
	ALIGN_AS(16) VECTOR3 color;
	ALIGN_AS(4) float intencity;
	ALIGN_AS(4) float lightAngleScale;
	ALIGN_AS(4) float lightAngleOffset;
	ALIGN_AS(4) int toTarget;
	ALIGN_AS(4) int shadowIndex;
	ALIGN_AS(4) int castShadows;
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
	ALIGN_AS(4) uint32_t alphaMode;
	ALIGN_AS(4) float alphaCutoff;

	ALIGN_AS(4) uint32_t baseColorTexture;
	ALIGN_AS(16) VECTOR4 baseColorFactor;

	ALIGN_AS(4) uint32_t metallicRoughnessTexture;
	ALIGN_AS(4) float metallicFactor;
	ALIGN_AS(4) float roughnessFactor;

	ALIGN_AS(4) uint32_t normalTexture;
	ALIGN_AS(4) float normalScale;

	ALIGN_AS(4) uint32_t occlusionTexture;
	ALIGN_AS(4) float occlusionStrenth;

	ALIGN_AS(4) uint32_t emissiveTexture;
	ALIGN_AS(16) VECTOR3 emissiveFactor;

	ALIGN_AS(4) uint32_t doubleSided;

	// KHR_materials_clearcoat
	ALIGN_AS(4) float clearcoatFactor;
	ALIGN_AS(4) float clearcoatRoughnessFactor;

	// KHR_materials_emissive_strength
	ALIGN_AS(4) float emissiveStrength;

	// KHR_materials_ior
	ALIGN_AS(4) float ior;

	// KHR_materials_iridescence
	ALIGN_AS(4) float iridescenceFactor;
	ALIGN_AS(4) float iridescenceIor;
	ALIGN_AS(4) float iridescenceThicknessMinimum;
	ALIGN_AS(4) float iridescenceThicknessMaximum;

	// KHR_materials_sheen
	ALIGN_AS(16) VECTOR3 sheenColorFactor;
	ALIGN_AS(4) float sheenRoughnessFactor;

	// KHR_materials_specular
	ALIGN_AS(4) float specularFactor;
	ALIGN_AS(16) VECTOR3 specularColorFactor;

	// KHR_materials_transmission
	ALIGN_AS(4) float transmissionFactor;

	// KHR_materials_unlit ?

	// KHR_materials_volume
	ALIGN_AS(4) float thicknessFactor;
	ALIGN_AS(4) float attenuationDistance;
	ALIGN_AS(16) VECTOR3 attenuationColor;
};

#ifdef __cplusplus
	}
}
#endif

#endif
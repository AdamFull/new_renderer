#ifndef SHLIB_H
#define SHLIB_H

#include "constants.h"
#include "scene.h"
#include "geometry.h"

SHLIB_NS_BEGIN

CONSTANT uint32_t SRV_TEXTURE_SLOT = 2;
CONSTANT uint32_t SAMPLER_SLOT = 3;

struct GPUBufferDesc
{
	uint64_t address;
	uint64_t size;
};

// Material
struct Material
{
#ifdef __cplusplus
	Material() :
		alphaMode{}, alphaCutoff{ 0.5f },
		base_color_texture{ ~0u }, base_color_sampler{ ~0u }, baseColorFactor{ 1.0f },
		metallic_roughness_texture{ ~0u }, metallic_roughness_sampler{ ~0u }, metallicFactor{ 1.f }, roughnessFactor{ 1.f },
		normal_texture{ ~0u }, normal_sampler{ ~0u }, normalScale{ 1.f },
		occlusion_texture{ ~0u }, occlusion_sampler{ ~0u }, occlusionStrenth{ 1.f },
		emissive_texture{ ~0u }, emissive_sampler{ ~0u }, emissiveFactor{ 0.f },
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

	uint32_t base_color_texture;
	uint32_t base_color_sampler;
	ALIGN_AS(16) vec4 baseColorFactor;

	uint32_t metallic_roughness_texture;
	uint32_t metallic_roughness_sampler;
	float metallicFactor;
	float roughnessFactor;

	uint32_t normal_texture;
	uint32_t normal_sampler;
	float normalScale;

	uint32_t occlusion_texture;
	uint32_t occlusion_sampler;
	float occlusionStrenth;

	uint32_t emissive_texture;
	uint32_t emissive_sampler;
	ALIGN_AS(16) vec3 emissiveFactor;

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
	ALIGN_AS(16) vec3 sheenColorFactor;
	float sheenRoughnessFactor;

	// KHR_materials_specular
	float specularFactor;
	ALIGN_AS(16) vec3 specularColorFactor;

	// KHR_materials_transmission
	float transmissionFactor;

	// KHR_materials_unlit ?

	// KHR_materials_volume
	float thicknessFactor;
	float attenuationDistance;
	ALIGN_AS(16) vec3 attenuationColor;
};

REGISTER_TYPE_PTR(Material, 16);

CB_BEGIN(GlobalUniformData, 0, 0)
	float dt;
	uint32_t directionalLightCount;
	uint32_t pointLightCount;
	uint32_t spotLightCount;
	uint32_t lightCount;
	uint32_t camera_count;
	uint32_t draw_count;
	uint32_t mesh_count;
	uint32_t defaultCameraID;
	uint32_t frameNumber;
CB_END(frameData)

CB_BEGIN(GlobalBufferReference, 0, 1)
	GET_PTR_T(mat4) matrixBuffer;
	GET_PTR_T(Mesh) meshBuffer;
	GET_PTR_T(SubMesh) submeshBuffer;

	GET_PTR_T(Material) materialBuffer;

	GET_PTR_T(LightCommit) lightsBuffer;

	GET_PTR_T(CascadedShadowCommit) cascadedShadowBuffer;
	GET_PTR_T(OmniShadowCommit) omniShadowBuffer;
	GET_PTR_T(SpotShadowCommit) spotShadowBuffer;

	GET_PTR_T(vec4) frustumSidesBuffer;

	GET_PTR_T(Camera) cameraBuffer;
CB_END_E()

SHLIB_NS_END

#endif
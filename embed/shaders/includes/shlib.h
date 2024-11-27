#ifndef SHLIB_H
#define SHLIB_H

#include "constants.h"
#include "scene.h"
#include "geometry.h"

SHLIB_NS_BEGIN

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
	ALIGN_AS(16) vec4 baseColorFactor;

	uint32_t metallicRoughnessTexture;
	float metallicFactor;
	float roughnessFactor;

	uint32_t normalTexture;
	float normalScale;

	uint32_t occlusionTexture;
	float occlusionStrenth;

	uint32_t emissiveTexture;
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
	uint32_t defaultCameraID;
	uint32_t frameNumber;
CB_END(frameData)

CB_BEGIN(GlobalBufferReference, 0, 1)
	GET_PTR_T(SceneNode) nodeBuffer;
	GET_PTR_T(mat4) matrixBuffer;
	GET_PTR_T(Mesh) meshBuffer;

	GET_PTR_T(Material) materialBuffer;

	GET_PTR_T(LightCommit) lightsBuffer;

	GET_PTR_T(CascadedShadowCommit) cascadedShadowBuffer;
	GET_PTR_T(OmniShadowCommit) omniShadowBuffer;
	GET_PTR_T(SpotShadowCommit) spotShadowBuffer;

	GET_PTR_T(vec4) frustumSidesBuffer;

	GET_PTR_T(Camera) cameraBuffer;
CB_END_E()

CB_BEGIN(GlobalPostProcessingData, 0, 2)
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
	ALIGN_AS(16) vec3 fog_color;
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
CB_END(postprocess)

SHLIB_NS_END

#endif
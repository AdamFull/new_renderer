#ifndef SHLIB_H
#define SHLIB_H

#include "constants.h"
#include "ptr.h"
#include "geometry.h"

SHLIB_NS_BEGIN

struct FGPUBufferDesc
{
	uint64_t address;
	uint64_t size;
};

struct FCamera
{
	uint32_t type;
	uint32_t matrices_offset;
	uint32_t frustum_offset;
	uint32_t last_hash;

	float fov;
	float z_near;
	float z_far;
	float aspect;

	vec2 size;
	vec2 invSize;
};

struct FGeometryDrawBounds
{
#ifdef __cplusplus
	FGeometryDrawBounds() : min{ (std::numeric_limits<float>::max)() }, max{ -(std::numeric_limits<float>::max)() } {}
#endif
	vec3 min;
	uint32_t pad0;
	vec3 max;
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

struct DrawDescription
{
	uint32_t vboID;
	uint32_t meshID;
	uint32_t nodeID;
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
	ALIGN_AS(16) vec3 position;
	ALIGN_AS(16) vec3 direction;
	ALIGN_AS(16) vec3 color;
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

CB_BEGIN(FGlobalUniformData, 0, 0)
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
CB_END(frameData)

// Base buffers
REGISTER_ARRAY_PTR(FSceneNode, 8);
REGISTER_ARRAY_PTR(FMesh, 8);
REGISTER_ARRAY_PTR(FMaterial, 8);

// Light buffers
REGISTER_ARRAY_PTR(FLightCommit, 8);

// Shadow buffers
REGISTER_ARRAY_PTR(FCascadedShadowCommit, 8);
REGISTER_ARRAY_PTR(FOmniShadowCommit, 8);
REGISTER_ARRAY_PTR(FSpotShadowCommit, 8);

// Vertex buffer specific data
REGISTER_ARRAY_PTR(FMeshlet, 8);
REGISTER_ARRAY_PTR(FGeometryDrawBounds, 8);
REGISTER_ARRAY_PTR(FTriangle, 8);
REGISTER_ARRAY_PTR(FMeshLod, 8);

// GPU object representation
REGISTER_ARRAY_PTR(DrawDescription, 8);

// Indirect data
REGISTER_ARRAY_PTR(FDrawIndexedIndirectData, 8);

REGISTER_ARRAY_PTR(FCamera, 8);


CB_BEGIN(FGlobalBufferReference, 0, 1)
	GET_ARR_PTR_T(FSceneNode) nodeBuffer;
	GET_ARR_PTR_T(mat4) matrixBuffer;
	GET_ARR_PTR_T(FMesh) meshBuffer;

	GET_ARR_PTR_T(FMaterial) materialBuffer;

	GET_ARR_PTR_T(FLightCommit) lightsBuffer;

	GET_ARR_PTR_T(FCascadedShadowCommit) cascadedShadowBuffer;
	GET_ARR_PTR_T(FOmniShadowCommit) omniShadowBuffer;
	GET_ARR_PTR_T(FSpotShadowCommit) spotShadowBuffer;

	GET_ARR_PTR_T(DrawDescription) drawInfoBuffer;
	GET_ARR_PTR_T(DrawDescription) rawDrawInfoBuffer;

	GET_ARR_PTR_T(vec4) frustumSidesBuffer;

	GET_ARR_PTR_T(FDrawIndexedIndirectData) indirectCommandBuffer;
	GET_ARR_PTR_T(uint32_t) indirectCountBuffer;

	GET_ARR_PTR_T(FCamera) cameraBuffer;
CB_END_E()

CB_BEGIN(FGlobalPostProcessingData, 0, 2)
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
#ifndef SHLIB_H
#define SHLIB_H

#include "constants.h"
#include "ptr.h"
#include "geometry.h"

SHLIB_NS_BEGIN

struct GPUBufferDesc
{
	uint64_t address;
	uint64_t size;
};

struct Camera
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

struct GeometryDrawBounds
{
#ifdef __cplusplus
	GeometryDrawBounds() : min{ (std::numeric_limits<float>::max)() }, max{ -(std::numeric_limits<float>::max)() } {}
#endif
	ALIGN_AS(16) vec3 min;
	ALIGN_AS(16) vec3 max;
};

struct MeshLod
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

struct Mesh
{
	uint32_t materialID;

	uint32_t lodOffset;
	uint32_t lodCount;

	uint32_t pad;

	GeometryDrawBounds bounds;
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
struct GeometryDrawRange
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

struct VertexRangeData
{
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
};

struct DrawIndexedIndirectData
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t  vertexOffset;
	uint32_t firstInstance;
};

struct DrawDescription
{
	uint32_t vboID;
	uint32_t meshID;
	uint32_t nodeID;
	uint32_t pad;
};

// Lights and shadows
struct CascadedShadowCommit
{
	ARRAY_OF(splitDepths, float, cascade_shadow_map_cascade_count);
	uint32_t cameraID;
	float farClip;
	int index;
	uint32_t pad;
};

struct LightCommit
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

struct OmniShadowCommit
{
	uint32_t cameraID;
	float farPlane;
	int index;
	uint32_t pad;
};

struct SpotShadowCommit
{
	uint32_t cameraID;
	int index;
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

CB_BEGIN(GlobalUniformData, 0, 0)
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
REGISTER_TYPE_PTR(FSceneNode, 8);
REGISTER_TYPE_PTR(Mesh, 8);
REGISTER_TYPE_PTR(Material, 8);

// Light buffers
REGISTER_TYPE_PTR(LightCommit, 8);

// Shadow buffers
REGISTER_TYPE_PTR(CascadedShadowCommit, 8);
REGISTER_TYPE_PTR(OmniShadowCommit, 8);
REGISTER_TYPE_PTR(SpotShadowCommit, 8);

// Vertex buffer specific data
//REGISTER_ARRAY_PTR(FMeshlet, 8);
//REGISTER_ARRAY_PTR(GeometryDrawBounds, 8);
//REGISTER_ARRAY_PTR(FTriangle, 8);
//REGISTER_ARRAY_PTR(MeshLod, 8);

// GPU object representation
REGISTER_TYPE_PTR(DrawDescription, 8);

// Indirect data
REGISTER_ARRAY_PTR(DrawIndexedIndirectData, 8);

REGISTER_TYPE_PTR(Camera, 8);


CB_BEGIN(GlobalBufferReference, 0, 1)
	GET_PTR_T(FSceneNode) nodeBuffer;
	GET_PTR_T(mat4) matrixBuffer;
	GET_PTR_T(Mesh) meshBuffer;

	GET_PTR_T(Material) materialBuffer;

	GET_PTR_T(LightCommit) lightsBuffer;

	GET_PTR_T(CascadedShadowCommit) cascadedShadowBuffer;
	GET_PTR_T(OmniShadowCommit) omniShadowBuffer;
	GET_PTR_T(SpotShadowCommit) spotShadowBuffer;

	GET_PTR_T(DrawDescription) drawInfoBuffer;
	GET_PTR_T(DrawDescription) rawDrawInfoBuffer;

	GET_PTR_T(vec4) frustumSidesBuffer;

	GET_ARR_PTR_T(DrawIndexedIndirectData) indirectCommandBuffer;
	GET_PTR_T(uint32_t) indirectCountBuffer;

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
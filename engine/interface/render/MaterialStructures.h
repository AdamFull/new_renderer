#pragma once 

#include <glm/glm.hpp>
#include <string>
#include <set>
#include <unordered_map>

namespace engine
{
	namespace render
	{
		enum EAlphaMode
		{
			MATERIAL_APLHA_MODE_OPAQUE,
			MATERIAL_APLHA_MODE_MASK,
			MATERIAL_APLHA_MODE_BLEND
		};

		struct FMaterial
		{
			// Alpha
			alignas(4) uint32_t alphaMode{};
			alignas(4) float alphaCutoff{ 0.5f };

			alignas(4) uint32_t baseColorTexture{ ~0u };
			alignas(16) glm::vec4 baseColorFactor{ 1.0f };

			alignas(4) uint32_t metallicRoughnessTexture{ ~0u };
			alignas(4) float metallicFactor{ 1.f };
			alignas(4) float roughnessFactor{ 1.f };

			alignas(4) uint32_t normalTexture{ ~0u };
			alignas(4) float normalScale{ 1.f };

			alignas(4) uint32_t occlusionTexture{ ~0u };
			alignas(4) float occlusionStrenth{ 1.f };

			alignas(4) uint32_t emissiveTexture{ ~0u };
			alignas(16) glm::vec3 emissiveFactor{ 0.f };
			
			alignas(4) uint32_t doubleSided{ false };

			// KHR_materials_clearcoat
			alignas(4) float clearcoatFactor{ 0.f };
			alignas(4) float clearcoatRoughnessFactor{ 0.f };

			// KHR_materials_emissive_strength
			alignas(4) float emissiveStrength{ 1.f };

			// KHR_materials_ior
			alignas(4) float ior{ 1.5f };

			// KHR_materials_iridescence
			alignas(4) float iridescenceFactor{ 0.f };
			alignas(4) float iridescenceIor{ 1.3f };
			alignas(4) float iridescenceThicknessMinimum{ 100.f };
			alignas(4) float iridescenceThicknessMaximum{ 400.f };

			// KHR_materials_sheen
			alignas(16) glm::vec3 sheenColorFactor{ 0.f };
			alignas(4) float sheenRoughnessFactor{ 0.f };

			// KHR_materials_specular
			alignas(4) float specularFactor{ 1.f };
			alignas(16) glm::vec3 specularColorFactor{ 1.f };


			// KHR_materials_transmission
			alignas(4) float transmissionFactor{ 0.f };

			// KHR_materials_unlit ?

			// KHR_materials_volume
			alignas(4) float thicknessFactor{ 0.f };
			alignas(4) float attenuationDistance{ INFINITY };
			alignas(16) glm::vec3 attenuationColor{ 1.f };
		};

		struct FMaterialCreateInfo
		{
			std::string shaderName{};
			FMaterial parameters{};
			size_t usageCount{ 0ull };
		};
	}
}
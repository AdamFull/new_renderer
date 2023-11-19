#pragma once 

#include <glm/glm.hpp>
#include <string>
#include <set>
#include <unordered_map>

namespace engine
{
	namespace render
	{
		enum class EAlphaMode
		{
			EOPAQUE,
			EMASK,
			EBLEND
		};

		struct FMaterial
		{
			// Base params
			EAlphaMode alphaMode{ EAlphaMode::EOPAQUE };
			float alphaCutoff{ 0.5f };
			glm::vec3 emissiveFactor{ 0.f };
			float normalScale{ 1.f };
			float occlusionStrenth{ 1.f };
			glm::vec4 baseColorFactor{ 1.0f };
			float metallicFactor{ 1.f };
			float roughnessFactor{ 1.f };
			std::set<std::string> vCompileDefinitions;
			bool doubleSided{ false };

			// KHR_materials_clearcoat
			float clearcoatFactor{ 0.f };
			float clearcoatRoughnessFactor{ 0.f };

			// KHR_materials_emissive_strength
			float emissiveStrength{ 1.f };

			// KHR_materials_ior
			float ior{ 1.5f };

			// KHR_materials_iridescence
			float iridescenceFactor{ 0.f };
			float iridescenceIor{ 1.3f };
			float iridescenceThicknessMinimum{ 100.f };
			float iridescenceThicknessMaximum{ 400.f };

			// KHR_materials_sheen
			glm::vec3 sheenColorFactor{ 0.f };
			float sheenRoughnessFactor{ 0.f };

			// KHR_materials_specular
			float specularFactor{ 1.f };
			glm::vec3 specularColorFactor{ 1.f };


			// KHR_materials_transmission
			float transmissionFactor{ 0.f };

			// KHR_materials_unlit ?

			// KHR_materials_volume
			float thicknessFactor{ 0.f };
			float attenuationDistance{ INFINITY };
			glm::vec3 attenuationColor{ 1.f };
		};

		struct FMaterialCreateInfo
		{
			std::string baseShaderName{};
			std::set<std::string> definitions{};
			std::unordered_map<std::string, size_t> textures{};
			size_t usageCount{ 0ull };

			FMaterial parameters{};
		};
	}
}
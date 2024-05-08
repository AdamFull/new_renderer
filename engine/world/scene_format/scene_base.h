#pragma once 

#include <standart_library.h>
#include <nlohmann/json.hpp>

namespace engine
{
	namespace world
	{
		namespace defaults
		{
			constexpr const u32 version{ 0x01010001 };
			constexpr const u32 invalid_index{ ~0u };
		}

		enum class EPrimitiveTopology
		{
			ePoints,
			eLine,
			eLineLoop,
			eLineStrip,
			eTriangles,
			eTriangleStrip,
			eTriangleFan
		};

		enum class EAttributeType
		{
			ePosition,
			eNormal,
			eTexCoord0,
			eTexCoord1,
			eColor0,
			eColor1,
			eJoints,
			eWeights,
			eCustom
		};

		struct FVersion
		{
			union {
				u32 code;
				struct {
					u8 major;
					u8 minor;
					u16 patch;
				};
			};

			bool operator==(const FVersion& other) const
			{
				return this->code == other.code;
			}
		};

		struct FAsset
		{
			FVersion version{};
			std::string description;

			bool operator==(const FAsset& other) const
			{
				return this->version == other.version && this->description == other.description;
			}
		};

		struct FPrimitiveLod
		{
			utl::hash_map<EAttributeType, int32_t> attributes; // (required)
			utl::vector<utl::hash_map<EAttributeType, int32_t>> targets;

			bool operator==(const FPrimitiveLod& other) const
			{
				return this->attributes == other.attributes && this->targets == other.targets;
			}
		};

		struct FPrimitive
		{
			utl::vector<FPrimitiveLod> lods{};

			int32_t material{ defaults::invalid_index };
			int32_t indices{ defaults::invalid_index };
			EPrimitiveTopology mode{ EPrimitiveTopology::eTriangleStrip };

			bool operator==(const FPrimitive& other) const
			{
				return
					this->lods == other.lods && this->material == other.material &&
					this->indices == other.indices &&
					this->mode == other.mode;
			}
		};

		struct FMesh
		{
			std::string name;
			utl::vector<FPrimitive> primitives; // (required)
			utl::vector<float> weights;

			bool operator==(const FMesh& other) const
			{
				return this->name == other.name && this->primitives == other.primitives;
			}
		};

		struct FNode
		{
			std::string name;
			// components
		};
		
		// Base structure with scene data
		struct FSceneData
		{
			FAsset asset{};

			utl::vector<FMesh> meshes{};
		};
	}
}
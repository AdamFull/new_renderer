#pragma once

#include <cmath>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "json.hpp"

namespace easygltf
{
	using dvec2 = std::array<double, 2>;
	using dvec3 = std::array<double, 3>;
	using dvec4 = std::array<double, 4>;
	using dmat4 = std::array<double, 16>;

	static bool compare_double(const double& lhs, const double& rhs)
	{
		constexpr auto epsilon = std::numeric_limits<double>::epsilon();
		return std::fabs(lhs - rhs) < epsilon;
	}

	static bool compare_dvec2(const dvec2& lhs, const dvec2& rhs)
	{
		return compare_double(lhs[0], rhs[0]) && compare_double(lhs[1], rhs[1]);
	}

	static bool compare_dvec3(const dvec3& lhs, const dvec3& rhs)
	{
		return compare_double(lhs[0], rhs[0]) && compare_double(lhs[1], rhs[1]) && compare_double(lhs[2], rhs[2]);
	}

	static bool compare_dvec4(const dvec4& lhs, const dvec4& rhs)
	{
		return compare_double(lhs[0], rhs[0]) && compare_double(lhs[1], rhs[1]) && compare_double(lhs[2], rhs[2]) && compare_double(lhs[3], rhs[3]);
	}

	static bool compare_dmat4(const dmat4& lhs, const dmat4& rhs)
	{
		return
			compare_double(lhs[0], rhs[0]) && compare_double(lhs[1], rhs[1]) && compare_double(lhs[2], rhs[2]) && compare_double(lhs[3], rhs[3]) &&
			compare_double(lhs[4], rhs[4]) && compare_double(lhs[5], rhs[5]) && compare_double(lhs[6], rhs[6]) && compare_double(lhs[7], rhs[7]) &&
			compare_double(lhs[8], rhs[8]) && compare_double(lhs[9], rhs[9]) && compare_double(lhs[10], rhs[10]) && compare_double(lhs[11], rhs[11]) &&
			compare_double(lhs[12], rhs[12]) && compare_double(lhs[13], rhs[13]) && compare_double(lhs[14], rhs[14]) && compare_double(lhs[15], rhs[15]);
	}

	static bool compare_dvector(const std::vector<double>& lhs, const std::vector<double>& rhs)
	{
		return (lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), compare_double));
	}

	namespace base
	{
		namespace enumerators
		{
			namespace primitive_topology
			{
				constexpr const int32_t points{ 0 };
				constexpr const int32_t line{ 1 };
				constexpr const int32_t line_loop{ 2 };
				constexpr const int32_t line_strip{ 3 };
				constexpr const int32_t triangles{ 4 };
				constexpr const int32_t triangle_strip{ 5 };
				constexpr const int32_t triangle_fan{ 6 };
			}

			namespace component_type
			{
				constexpr const int32_t byte_type{ 5120 };
				constexpr const int32_t ubyte_type{ 5121 };
				constexpr const int32_t short_type{ 5122 };
				constexpr const int32_t ushort_type{ 5123 };
				constexpr const int32_t int_type{ 5124 };
				constexpr const int32_t uint_type{ 5125 };
				constexpr const int32_t float_type{ 5126 };
				constexpr const int32_t double_type{ 5130 };

				constexpr const int32_t float_vec2_type{ 35664 };
				constexpr const int32_t float_vec3_type{ 35665 };
				constexpr const int32_t float_vec4_type{ 35666 };

				constexpr const int32_t int_vec2_type{ 35667 };
				constexpr const int32_t int_vec3_type{ 35668 };
				constexpr const int32_t int_vec4_type{ 35669 };

				constexpr const int32_t bool_type{ 35670 };
				constexpr const int32_t bool_vec2_type{ 35671 };
				constexpr const int32_t bool_vec3_type{ 35672 };
				constexpr const int32_t bool_vec4_type{ 35673 };

				constexpr const int32_t float_mat2_type{ 35674 };
				constexpr const int32_t float_mat3_type{ 35675 };
				constexpr const int32_t float_mat4_type{ 35676 };
			}

			namespace texture_filter
			{
				constexpr const int32_t nearest{ 9728 };
				constexpr const int32_t linear{ 9729 };
				constexpr const int32_t nearest_mipmap_nearest{ 9984 };
				constexpr const int32_t linear_mipmap_nearest{ 9985 };
				constexpr const int32_t nearest_mipmap_linear{ 9986 };
				constexpr const int32_t linear_mipmap_linear{ 9987 };
			}

			namespace texture_wrap
			{
				constexpr const int32_t repeat{ 9728 };
				constexpr const int32_t clamp_to_edge{ 9728 };
				constexpr const int32_t mirrored_repeat{ 9728 };
			}

			namespace camera_type
			{
				constexpr const char* perspective{ "perspective" };
				constexpr const char* orthographic{ "orthographic" };
			}

			namespace alpha_mode
			{
				constexpr const char* opaque{ "OPAQUE" };
				constexpr const char* mask{ "MASK" };
				constexpr const char* blend{ "BLEND" };
			}

			namespace animation_path
			{
				constexpr const char* translation{ "translation" };
				constexpr const char* rotation{ "rotation" };
				constexpr const char* scale{ "scale" };
				constexpr const char* weights{ "weights" };
			}

			namespace interpolation
			{
				constexpr const char* linear{ "LINEAR" };
				constexpr const char* step{ "STEP" };
				constexpr const char* cubicspline{ "CUBICSPLINE" };
			}
		}

		namespace constants
		{
			constexpr const char* spec_version{ "2.0" };

			constexpr const int32_t not_set{ -1 };

			constexpr const double dzero{ 0.0 };
			constexpr const double done{ 0.0 };

			constexpr const dvec2 dvec2zeros{ 0.0, 0.0 };
			constexpr const dvec2 dvec2ones{ 1.0, 1.0 };
			constexpr const dvec3 dvec3zeros{ 0.0, 0.0, 0.0 };
			constexpr const dvec3 dvec3ones{ 1.0, 1.0, 1.0 };
			constexpr const dvec4 dvec4zeros{ 0.0, 0.0, 0.0, 0.0 };
			constexpr const dvec4 dvec4ones{ 1.0, 1.0, 1.0, 1.0 };
			constexpr const dvec4 dvec4quat_default{ 0.0, 0.0, 0.0, 1.0 };
			constexpr const dmat4 dmat4identity{ 1.0, 0.0, 0.0, 0.0,
												0.0, 1.0, 0.0, 0.0,
												0.0, 0.0, 1.0, 0.0,
												0.0, 0.0, 0.0, 1.0 };
		}
	}

	struct Default
	{
		Default() = default;
		~Default() = default;
		Default(const Default&) = default;
		Default(Default&&) noexcept = default;
		Default& operator=(const Default&) = default;
		Default& operator=(Default&&) noexcept = default;

		const bool hasExtension(const std::string& extension_name) const
		{
			return extensions.count(extension_name);
		}

		bool compareExtensionsAndExtras(const Default& other) const
		{
			return this->extensions == other.extensions && this->extras == other.extras;
		}

		mutable std::unordered_map<std::string, nlohmann::json> extensions;
		mutable std::unordered_map<std::string, nlohmann::json> extras;
	};

	namespace extensions
	{
		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_transform
		struct KHR_texture_transform
		{
			static constexpr const char* name{ "KHR_texture_transform" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/schema/KHR_texture_transform.textureInfo.schema.json
			struct TextureTransform : Default
			{
				dvec2 offset{ base::constants::dvec2zeros };
				double rotation{ base::constants::dzero };
				dvec2 scale{ base::constants::dvec2ones };
				int32_t texCoord{ base::constants::not_set };

				bool operator==(const TextureTransform& other) const
				{
					return
						compare_dvec2(this->offset, other.offset) && compare_double(this->rotation, other.rotation) &&
						compare_dvec2(this->scale, other.scale) && this->texCoord == other.texCoord &&
						compareExtensionsAndExtras(other);
				}
			};

		};
	}


	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/textureInfo.schema.json
	struct TextureInfo : public Default
	{
		int32_t index{ base::constants::not_set }; // (required)
		int32_t texCoord{ base::constants::not_set };

		extensions::KHR_texture_transform::TextureTransform khrTextureTransform{};

		bool operator==(const TextureInfo& other) const
		{
			return
				this->index == other.index && this->texCoord == other.texCoord &&
				this->khrTextureTransform == other.khrTextureTransform && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.normalTextureInfo.schema.json
	struct NormalTexture : public TextureInfo
	{
		double scale{ base::constants::done };

		bool operator==(const NormalTexture& other) const
		{
			return
				this->index == other.index && this->texCoord == other.texCoord &&
				compare_double(this->scale, other.scale) && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.occlusionTextureInfo.schema.json
	struct OcclusionTexture : public TextureInfo
	{
		double strength{ base::constants::done };

		bool operator==(const OcclusionTexture& other) const
		{
			return
				this->index == other.index && this->texCoord == other.texCoord &&
				compare_double(this->strength, other.strength) && compareExtensionsAndExtras(other);
		}
	};

	struct CubemapTexture : TextureInfo
	{
		// This is optional. If you using ktx texture you can set single index
		int32_t positiveX{ base::constants::not_set };
		int32_t negativeX{ base::constants::not_set };
		int32_t positiveY{ base::constants::not_set };
		int32_t negativeY{ base::constants::not_set };
		int32_t positiveZ{ base::constants::not_set };
		int32_t negativeZ{ base::constants::not_set };

		bool operator==(const CubemapTexture& other) const
		{
			return
				this->index == other.index && this->texCoord == other.texCoord &&
				this->positiveX == other.positiveX && this->negativeX == other.negativeX && 
				this->positiveY == other.positiveY && this->negativeY == other.negativeY &&
				this->positiveZ == other.positiveZ && this->negativeZ == other.negativeZ &&
				compareExtensionsAndExtras(other);
		}
	};
	
	namespace extensions
	{
		// User extenstions
		struct EXT_materials_displacement
		{
			static constexpr const char* name{ "EXT_materials_displacement" };

			struct MaterialsDisplacement : Default
			{
				TextureInfo displacementTexture{};
				double tessellationFactor{ base::constants::done };
				double displacementStrength{ base::constants::done };

				bool operator==(const MaterialsDisplacement& other) const
				{
					return
						this->displacementTexture == other.displacementTexture && compare_double(this->tessellationFactor, other.tessellationFactor) &&
						compare_double(this->displacementStrength, other.displacementStrength) && compareExtensionsAndExtras(other);
				}
			};
		};

		struct EXT_lights_shadows
		{
			static constexpr const char* name{ "EXT_lights_shadows" };

			struct CastShadows : Default
			{
				bool castShadows{ false };

				bool operator==(const CastShadows& other) const
				{
					return
						this->castShadows == other.castShadows && compareExtensionsAndExtras(other);
				}
			};
		};

		struct EXT_scenes_environment_ibl
		{
			static constexpr const char* name{ "EXT_scenes_environment_ibl" };

			struct IBLEnvironmentDesc : Default
			{
				CubemapTexture environmentTexture{};
				double intensity{ 1.0 };

				bool operator==(const IBLEnvironmentDesc& other) const
				{
					return
						this->environmentTexture == other.environmentTexture && compare_double(this->intensity, other.intensity) &&
						compareExtensionsAndExtras(other);
				}
			};

			struct IBLEnvironments : Default
			{
				int32_t index{ base::constants::not_set };
				std::vector<IBLEnvironmentDesc> environments{};

				bool operator==(const IBLEnvironments& other) const
				{
					return
						this->index == other.index && this->environments == other.environments && 
						compareExtensionsAndExtras(other);
				}
			};
		};

		// Official KHR extensions
		struct KHR_audio
		{
			// https://github.com/omigroup/glTF/tree/KHR_audio/extensions/2.0/Khronos/KHR_audio
			static constexpr const char* name{ "KHR_audio" };

			struct constants
			{
				static constexpr const double cone_angle_default{ 6.283185307179586 };
				static constexpr const double outer_gain_min{ base::constants::dzero };
				static constexpr const double outer_gain_max{ base::constants::done };
				static constexpr const double outer_gain_default{ base::constants::dzero };
				static constexpr const double max_distance_min{ base::constants::dzero };
				static constexpr const double max_distance_default{ 10000.0 };
				static constexpr const double ref_distance_min{ base::constants::dzero };
				static constexpr const double ref_distance_default{ base::constants::done };
				static constexpr const double rolloff_factor_min{ base::constants::dzero };
				static constexpr const double rolloff_factor_default{ base::constants::done };
			};

			struct enumerators
			{
				struct source_type
				{
					// Global audio emitters are not affected by the position of audio listeners. coneInnerAngle, coneOuterAngle, 
					// coneOuterGain, distanceModel, maxDistance, refDistance, and rolloffFactor should all be ignored when set.
					static constexpr const char* global{ "global" };
					// Positional audio emitters. Using sound cones, the orientation is +Z having the same front side for a glTF asset.
					static constexpr const char* positional{ "positional" };
				};

				struct distance_model
				{
					// A linear distance model calculating the gain induced by the distance according to: 
					// 1.0 - rolloffFactor * (distance - refDistance) / (maxDistance - refDistance)
					static constexpr const char* linear{ "linear" };
					//An inverse distance model calculating the gain induced by the distance according to: 
					// refDistance / (refDistance + rolloffFactor * (Math.max(distance, refDistance) - refDistance))
					static constexpr const char* inverse{ "inverse" };
					// An exponential distance model calculating the gain induced by the distance according to:
					// pow((Math.max(distance, refDistance) / refDistance, -rolloffFactor))
					static constexpr const char* exponential{ "exponential" };
				};
			};

			struct AudioNode : Default
			{
				int32_t emitter{ base::constants::not_set }; // (required)

				bool operator==(const AudioNode& other) const
				{
					return
						this->emitter == other.emitter && compareExtensionsAndExtras(other);
				}
			};

			struct AudioScene : Default
			{
				std::vector<int32_t> emitters{}; // (required)

				bool operator==(const AudioScene& other) const
				{
					return
						this->emitters == other.emitters && compareExtensionsAndExtras(other);
				}
			};

			struct PositionalEmitter : Default
			{
				double coneInnerAngle{ constants::cone_angle_default };
				double coneOuterAngle{ constants::cone_angle_default };
				double coneOuterGain{ constants::outer_gain_default };
				double maxDistance{ constants::max_distance_default };
				double refDistance{ constants::ref_distance_default };
				double rolloffFactor{ constants::rolloff_factor_default };

				bool operator==(const PositionalEmitter& other) const
				{
					return
						compare_double(this->coneInnerAngle, other.coneInnerAngle) && compare_double(this->coneOuterAngle, other.coneOuterAngle) &&
						compare_double(this->coneOuterGain, other.coneOuterGain) && compare_double(this->maxDistance, other.maxDistance) &&
						compare_double(this->refDistance, other.refDistance) && compare_double(this->rolloffFactor, other.rolloffFactor) &&
						compareExtensionsAndExtras(other);
				}
			};

			struct AudioEmitter : Default
			{
				std::string name{};
				double gain{ base::constants::done };
				bool loop{ false };
				bool playing{ false };
				std::string type{ enumerators::source_type::global };
				std::string distanceModel{ enumerators::distance_model::inverse };

				PositionalEmitter positional{};

				bool operator==(const AudioEmitter& other) const
				{
					return
						this->name == other.name && compare_double(this->gain, other.gain) &&
						this->loop == other.loop && this->playing == other.playing &&
						this->type == other.type && this->distanceModel == other.distanceModel &&
						this->positional == other.positional && compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/omigroup/glTF/blob/KHR_audio/extensions/2.0/Khronos/KHR_audio/schema/source.schema.json
			struct AudioSource : Default
			{
				std::string name{};
				std::string uri{};
				int32_t bufferView{ base::constants::not_set };
				std::string mimeType{};

				bool operator==(const AudioSource& other) const
				{
					return
						this->name == other.name && this->uri == other.uri &&
						this->bufferView == other.bufferView && this->mimeType == other.mimeType &&
						compareExtensionsAndExtras(other);
				}
			};

			struct Audio : Default
			{
				std::vector<AudioEmitter> emitters{};
				std::vector<AudioSource> sources{};

				bool operator==(const Audio& other) const
				{
					return
						this->emitters == other.emitters && this->sources == other.sources &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_draco_mesh_compression
		struct KHR_draco_mesh_compression
		{
			static constexpr const char* name{ "KHR_draco_mesh_compression" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_draco_mesh_compression/schema/mesh.primitive.KHR_draco_mesh_compression.schema.json
			struct DracoMeshCompression : Default
			{
				int32_t bufferView{ base::constants::not_set }; // (required)
				std::unordered_map<std::string, int32_t> attributes; // (required)

				bool operator==(const DracoMeshCompression& other) const
				{
					return
						this->bufferView == other.bufferView && this->attributes == other.attributes &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_anisotropy
		struct KHR_materials_anisotropy
		{
			static constexpr const char* name{ "KHR_materials_anisotropy" };

			struct constants
			{
				static constexpr const double anisotropy_strength_min{ base::constants::dzero };
				static constexpr const double anisotropy_strength_max{ base::constants::done };
				static constexpr const double anisotropy_strength_default{ anisotropy_strength_min };
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_anisotropy/schema/glTF.KHR_materials_anisotropy.schema.json
			struct MaterialsAnisotropy : Default
			{
				double anisotropyStrength{ constants::anisotropy_strength_default };
				double anisotropyRotation{ base::constants::dzero };
				TextureInfo anisotropyTexture{};

				bool operator==(const MaterialsAnisotropy& other) const
				{
					return
						compare_double(this->anisotropyStrength, other.anisotropyStrength) && compare_double(this->anisotropyRotation, other.anisotropyRotation) &&
						this->anisotropyTexture == other.anisotropyTexture && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat
		struct KHR_materials_clearcoat
		{
			static constexpr const char* name{ "KHR_materials_clearcoat" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_clearcoat/schema/glTF.KHR_materials_clearcoat.schema.json
			struct MaterialClearcoat : Default
			{
				double clearcoatFactor{ base::constants::dzero };
				TextureInfo clearcoatTexture{};
				double clearcoatRoughnessFactor{ base::constants::dzero };
				TextureInfo clearcoatRoughnessTexture{};
				NormalTexture clearcoatNormalTexture{};

				bool operator==(const MaterialClearcoat& other) const
				{
					return
						compare_double(this->clearcoatFactor, other.clearcoatFactor) && this->clearcoatTexture == other.clearcoatTexture &&
						compare_double(this->clearcoatRoughnessFactor, other.clearcoatRoughnessFactor) && this->clearcoatRoughnessTexture == other.clearcoatRoughnessTexture &&
						this->clearcoatNormalTexture == other.clearcoatNormalTexture && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_emissive_strength
		struct KHR_materials_emissive_strength
		{
			static constexpr const char* name{ "KHR_materials_emissive_strength" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_emissive_strength/schema/glTF.KHR_materials_emissive_strength.schema.json
			struct MaterialEmissiveStrength : Default
			{
				double emissiveStrength{ base::constants::done };

				bool operator==(const MaterialEmissiveStrength& other) const
				{
					return
						compare_double(this->emissiveStrength, other.emissiveStrength) && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_ior
		struct KHR_materials_ior
		{
			static constexpr const char* name{ "KHR_materials_ior" };

			struct constants
			{
				static constexpr const double ior_min{ base::constants::dzero };
				static constexpr const double ior_default{ 1.5 };
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_ior/schema/glTF.KHR_materials_ior.schema.json
			struct MaterialIor : Default
			{
				double ior{ constants::ior_default };

				bool operator==(const MaterialIor& other) const
				{
					return
						compare_double(this->ior, other.ior) && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_iridescence
		struct KHR_materials_iridescence
		{
			static constexpr const char* name{ "KHR_materials_iridescence" };

			struct constants
			{
				static constexpr const double factor_min{ base::constants::dzero };
				static constexpr const double factor_max{ base::constants::done };
				static constexpr const double factor_default{ factor_min };

				static constexpr const double ior_min{ base::constants::done };
				static constexpr const double ior_default{ 1.3 };

				static constexpr const double thickness_min_min{ base::constants::dzero };
				static constexpr const double thickness_min_default{ 100.0 };
				static constexpr const double thickness_max_min{ base::constants::dzero };
				static constexpr const double thickness_max_default{ 400.0 };
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_iridescence/schema/glTF.KHR_materials_iridescence.schema.json
			struct MaterialIridescence : Default
			{
				double iridescenceFactor{ constants::factor_default };
				TextureInfo iridescenceTexture{};
				double iridescenceIor{ constants::ior_default };
				double iridescenceThicknessMinimum{ constants::thickness_min_default };
				double iridescenceThicknessMaximum{ constants::thickness_max_default };
				TextureInfo iridescenceThicknessTexture{};

				bool operator==(const MaterialIridescence& other) const
				{
					return
						compare_double(this->iridescenceFactor, other.iridescenceFactor) && this->iridescenceTexture == other.iridescenceTexture &&
						compare_double(this->iridescenceIor, other.iridescenceIor) && compare_double(this->iridescenceThicknessMinimum, other.iridescenceThicknessMinimum) &&
						compare_double(this->iridescenceThicknessMaximum, other.iridescenceThicknessMaximum) && this->iridescenceThicknessTexture == other.iridescenceThicknessTexture &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_sheen
		struct KHR_materials_sheen
		{
			static constexpr const char* name{ "KHR_materials_sheen" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_sheen/schema/glTF.KHR_materials_sheen.schema.json
			struct MaterialSheen : Default
			{
				dvec3 sheenColorFactor{ base::constants::dvec3zeros };
				TextureInfo sheenColorTexture{};
				double sheenRoughnessFactor{ base::constants::dzero };
				TextureInfo sheenRoughnessTexture{};

				bool operator==(const MaterialSheen& other) const
				{
					return
						compare_dvec3(this->sheenColorFactor, other.sheenColorFactor) && this->sheenColorTexture == other.sheenColorTexture &&
						compare_double(this->sheenRoughnessFactor, other.sheenRoughnessFactor) && this->sheenRoughnessTexture == other.sheenRoughnessTexture &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_specular
		struct KHR_materials_specular
		{
			static constexpr const char* name{ "KHR_materials_specular" };

			struct constants
			{
				static constexpr const double factor_min{ base::constants::dzero };
				static constexpr const double factor_max{ base::constants::done };
				static constexpr const double factor_default{ base::constants::done };
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_specular/schema/glTF.KHR_materials_specular.schema.json
			struct MaterialSpecular : Default
			{
				double specularFactor{ constants::factor_default };
				TextureInfo specularTexture{};
				dvec3 specularColorFactor{ base::constants::dvec3ones };
				TextureInfo specularColorTexture{};

				bool operator==(const MaterialSpecular& other) const
				{
					return
						compare_double(this->specularFactor, other.specularFactor) && this->specularTexture == other.specularTexture &&
						compare_dvec3(this->specularColorFactor, other.specularColorFactor) && this->specularColorTexture == other.specularColorTexture &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_transmission
		struct KHR_materials_transmission
		{
			static constexpr const char* name{ "KHR_materials_transmission" };

			struct constants
			{
				static constexpr const double factor_min{ base::constants::dzero };
				static constexpr const double factor_max{ base::constants::done };
				static constexpr const double factor_default{ base::constants::dzero };
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_transmission/schema/glTF.KHR_materials_transmission.schema.json
			struct MaterialTransmission : Default
			{
				double transmissionFactor{ constants::factor_default };
				TextureInfo transmissionTexture{};

				bool operator==(const MaterialTransmission& other) const
				{
					return
						compare_double(this->transmissionFactor, other.transmissionFactor) && this->transmissionTexture == other.transmissionTexture &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_unlit
		struct KHR_materials_unlit
		{
			static constexpr const char* name{ "KHR_materials_unlit" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_unlit/schema/glTF.KHR_materials_unlit.schema.json
			struct MaterialUnlit : Default
			{
				bool operator==(const MaterialUnlit& other) const
				{
					return
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_variants
		struct KHR_materials_variants
		{
			static constexpr const char* name{ "KHR_materials_variants" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_variants/schema/glTF.KHR_materials_variants.schema.json
			struct Variant : Default
			{
				std::string name{};

				bool operator==(const Variant& other) const
				{
					return
						this->name == other.name && compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_variants/schema/glTF.KHR_materials_variants.schema.json
			struct MaterialVariants : Default
			{
				std::vector<Variant> variants{}; // (required)

				bool operator==(const MaterialVariants& other) const
				{
					return
						this->variants == other.variants && compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_variants/schema/mesh.primitive.KHR_materials_variants.schema.json
			struct Mapping : Default
			{
				std::string name{};
				std::vector<int32_t> variants{}; // (required)
				int32_t material{ base::constants::not_set }; // (required)

				bool operator==(const Mapping& other) const
				{
					return
						this->name == other.name && this->variants == other.variants &&
						this->material == other.material && compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_variants/schema/mesh.primitive.KHR_materials_variants.schema.json
			struct MaretialVariant : Default
			{
				std::vector<Mapping> mappings{}; // (required)

				bool operator==(const MaretialVariant& other) const
				{
					return
						this->mappings == other.mappings && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_volume
		struct KHR_materials_volume
		{
			static constexpr const char* name{ "KHR_materials_volume" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_volume/schema/glTF.KHR_materials_volume.schema.json
			struct MaterialVolume : Default
			{
				double thicknessFactor{ base::constants::dzero };
				TextureInfo thicknessTexture{};
				double attenuationDistance{ base::constants::dzero };
				dvec3 attenuationColor{ base::constants::dvec3ones };

				bool operator==(const MaterialVolume& other) const
				{
					return
						compare_double(this->thicknessFactor, other.thicknessFactor) && this->thicknessTexture == other.thicknessTexture &&
						compare_double(this->attenuationDistance, other.attenuationDistance) && compare_dvec3(this->attenuationColor, other.attenuationColor) &&
						compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_lights_punctual
		struct KHR_lights_punctual
		{
			static constexpr const char* name{ "KHR_lights_punctual" };

			struct constants
			{
				static constexpr const double cone_min{ base::constants::dzero };
				static constexpr const double cone_max{ 1.5707963267948966 };
				static constexpr const double inner_default{ cone_min };
				static constexpr const double outer_default{ 0.7853981633974483 };
			};

			struct enumerators
			{
				struct light_type
				{
					static constexpr const char* point{ "point" };
					static constexpr const char* spot{ "spot" };
					static constexpr const char* directional{ "directional" };
				};
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/schema/light.spot.schema.json
			struct SpotLight : Default
			{
				double innerConeAngle{ constants::inner_default }; // min 0.0, max 1.5707963267948966, default 0.0
				double outerConeAngle{ constants::outer_default }; // min 0.0, max , default 0.7853981633974483

				bool operator==(const SpotLight& other) const
				{
					return
						compare_double(this->innerConeAngle, other.innerConeAngle) && compare_double(this->outerConeAngle, other.outerConeAngle) &&
						compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/schema/light.schema.json
			struct Light : Default
			{
				std::string name{};
				dvec3 color{ base::constants::dvec3ones };
				double intensity{ base::constants::done };
				SpotLight spot{};
				std::string type{ enumerators::light_type::point };
				double range{ base::constants::dzero };

				EXT_lights_shadows::CastShadows extLightShadows{};

				bool operator==(const Light& other) const
				{
					return
						this->name == other.name && compare_dvec3(this->color, other.color) &&
						compare_double(this->intensity, other.intensity) && this->spot == other.spot &&
						this->type == other.type && compare_double(this->range, other.range) &&
						this->extLightShadows == other.extLightShadows && compareExtensionsAndExtras(other);
				}
			};

			struct LightsPunctual : Default
			{
				std::vector<Light> lights{}; // (required)

				bool operator==(const LightsPunctual& other) const
				{
					return
						this->lights == other.lights && compareExtensionsAndExtras(other);
				}
			};

			struct LightsPunctualNode : Default
			{
				int32_t light{ -1 }; // (required)

				bool operator==(const LightsPunctualNode& other) const
				{
					return
						this->light == other.light && compareExtensionsAndExtras(other);
				}
			};
		};

		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_basisu
		struct KHR_texture_basisu
		{
			static constexpr const char* name{ "KHR_texture_basisu" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/schema/KHR_texture_transform.textureInfo.schema.json
			struct TextureBasisU : Default
			{
				int32_t source{};

				bool operator==(const TextureBasisU& other) const
				{
					return
						this->source == other.source && compareExtensionsAndExtras(other);
				}
			};
		};


		// N/A
		// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_xmp_json_ld
		struct KHR_xmp_json_ld
		{
			static constexpr const char* name{ "KHR_xmp_json_ld" };

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_xmp_json_ld/schema/KHR_xmp_json_ld.schema.json
			struct XMP : Default
			{
				int32_t packet{ base::constants::not_set }; // (required)

				bool operator==(const XMP& other) const
				{
					return
						this->packet == other.packet && compareExtensionsAndExtras(other);
				}
			};

			// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_xmp_json_ld/schema/glTF.KHR_xmp_json_ld.schema.json
			struct XMP_json : Default
			{
				bool operator==(const XMP_json& other) const
				{
					return
						compareExtensionsAndExtras(other);
				}
			};
		};
	}

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.sparse.indices.schema.json
	struct SparseAccessorIndices : public Default
	{
		int32_t bufferView{ base::constants::not_set }; // (required)
		size_t byteOffset{ 0ull };
		int32_t componentType{ base::constants::not_set }; // (required)

		bool operator==(const SparseAccessorIndices& other) const
		{
			return
				this->bufferView == other.bufferView && this->byteOffset == other.byteOffset &&
				this->componentType == other.componentType && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.sparse.values.schema.json
	struct SparseAccessorValues : public Default
	{
		int32_t bufferView{ base::constants::not_set }; // (required)
		size_t byteOffset{ 0ull };

		bool operator==(const SparseAccessorValues& other) const
		{
			return
				this->bufferView == other.bufferView && this->byteOffset == other.byteOffset &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.sparse.schema.json
	struct SparseAccessor : public Default
	{
		int32_t count{ base::constants::not_set }; // (required)
		SparseAccessorIndices indices{}; // (required)
		SparseAccessorValues values{}; // (required)

		bool operator==(const SparseAccessor& other) const
		{
			return
				this->count == other.count && this->indices == other.indices &&
				this->values == other.values && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
	struct Accessor : public Default
	{
		std::string name{};
		int32_t bufferView{ base::constants::not_set };
		size_t byteOffset{ 0ull };
		size_t count{ 0ull }; // (required)
		bool normalized{ false };
		int32_t componentType{ base::constants::not_set }; // (required)
		std::string type{ }; // (required)

		std::vector<double> min{};
		std::vector<double> max{};

		SparseAccessor sparse{};

		bool operator==(const Accessor& other) const
		{
			return
				this->name == other.name && this->bufferView == other.bufferView &&
				this->byteOffset == other.byteOffset && this->count == other.count &&
				this->normalized == other.normalized && this->componentType == other.componentType &&
				this->type == other.type && compare_dvector(this->min, other.min) &&
				compare_dvector(this->max, other.max) && this->sparse == other.sparse &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/animation.channel.target.schema.json
	struct AnimationTarget : public Default
	{
		int32_t node{ base::constants::not_set };
		std::string path{}; // (required)

		bool operator==(const AnimationTarget& other) const
		{
			return
				this->node == other.node && this->path == other.path &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/animation.channel.schema.json
	struct AnimationChannel : public Default
	{
		int32_t sampler{ base::constants::not_set }; // (required)
		AnimationTarget target{}; // (required)

		bool operator==(const AnimationChannel& other) const
		{
			return
				this->sampler == other.sampler && this->target == other.target &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/animation.sampler.schema.json
	struct AnimationSampler : public Default
	{
		int32_t input{ base::constants::not_set }; // (required)
		int32_t output{ base::constants::not_set }; // (required)
		std::string interpolation{ base::enumerators::interpolation::linear };

		bool operator==(const AnimationSampler& other) const
		{
			return
				this->input == other.input && this->output == other.output &&
				this->interpolation == other.interpolation && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/animation.schema.json
	struct Animation : public Default
	{
		std::string name{};
		std::vector<AnimationChannel> channels; // (required)
		std::vector<AnimationSampler> samplers; // (required)

		bool operator==(const Animation& other) const
		{
			return
				this->name == other.name && this->channels == other.channels &&
				this->samplers == other.samplers && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
	struct Asset : public Default
	{
		std::string version{ base::constants::spec_version };  // (required)
		std::string generator;
		std::string minVersion;
		std::string copyright;

		bool operator==(const Asset& other) const
		{
			return
				this->version == other.version && this->generator == other.generator &&
				this->minVersion == other.minVersion && this->copyright == other.copyright &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
	struct Buffer : public Default
	{
		std::string name{};
		std::string uri{};
		size_t byteLength{ 0ull }; // (required)

		bool operator==(const Buffer& other) const
		{
			return
				this->name == other.name && this->uri == other.uri &&
				this->byteLength == other.byteLength && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
	struct BufferView : public Default
	{
		std::string name;
		int32_t buffer{ base::constants::not_set }; // (required)
		size_t byteOffset{ 0ull };
		size_t byteLength{ 0ull }; // (required)
		size_t byteStride{ 0ull };

		int32_t target{ 0 };

		bool operator==(const BufferView& other) const
		{
			return
				this->name == other.name && this->buffer == other.buffer &&
				this->byteOffset == other.byteOffset && this->byteLength == other.byteLength &&
				this->byteStride == other.byteStride && this->target == other.target &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/camera.perspective.schema.json
	struct PerspectiveCamera : public Default
	{
		double aspectRatio{ base::constants::dzero };
		double yfov{ 0.8 };         // (required)
		double zfar{ base::constants::dzero };
		double znear{ 0.001 };        // (required)

		bool operator==(const PerspectiveCamera& other) const
		{
			return
				compare_double(this->aspectRatio, other.aspectRatio) &&
				compare_double(this->yfov, other.yfov) &&
				compare_double(this->zfar, other.zfar) &&
				compare_double(this->znear, other.znear) &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/camera.orthographic.schema.json
	struct OrthographicCamera : public Default
	{
		double xmag{ base::constants::done };   // (required)
		double ymag{ base::constants::done };   // (required)
		double zfar{ base::constants::done };   // (required)
		double znear{ base::constants::dzero };  // (required)

		bool operator==(const OrthographicCamera& other) const
		{
			return
				compare_double(this->xmag, other.xmag) &&
				compare_double(this->ymag, other.ymag) &&
				compare_double(this->zfar, other.zfar) &&
				compare_double(this->znear, other.znear) &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/camera.schema.json
	struct Camera : public Default
	{
		std::string name;
		std::string type{ base::enumerators::camera_type::perspective };

		PerspectiveCamera perspective{}; // (required)
		OrthographicCamera orthographic{}; // (required)

		bool operator==(const Camera& other) const
		{
			return
				this->name == other.name && this->type == other.type &&
				this->perspective == other.perspective && this->orthographic == other.orthographic &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
	struct Image : public Default
	{
		std::string name{};
		int32_t bufferView{ base::constants::not_set }; // required if no uri (glb)
		std::string mimeType{}; // required if no uri (glb)
		std::string uri{}; // required if no bufferView and mimeType 

		bool operator==(const Image& other) const
		{
			return
				this->name == other.name && this->bufferView == other.bufferView &&
				this->mimeType == other.mimeType && this->uri == other.uri &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.pbrMetallicRoughness.schema.json
	// A set of parameter values that are used to define the metallic-roughness material model from Physically-Based Rendering (PBR) methodology.
	struct PbrMetallicRoughness : public Default
	{
		dvec4 baseColorFactor{ base::constants::dvec4ones };
		TextureInfo baseColorTexture{};
		double metallicFactor{ base::constants::done };
		double roughnessFactor{ base::constants::done };
		TextureInfo metallicRoughnessTexture{};

		bool operator==(const PbrMetallicRoughness& other) const
		{
			return
				this->baseColorFactor == other.baseColorFactor && this->baseColorTexture == other.baseColorTexture &&
				compare_double(this->metallicFactor, other.metallicFactor) && compare_double(this->roughnessFactor, other.roughnessFactor) &&
				this->metallicRoughnessTexture == other.metallicRoughnessTexture && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.schema.json
	struct Material : public Default
	{
		std::string name{};
		PbrMetallicRoughness pbrMetallicRoughness{};
		NormalTexture normalTexture{};
		OcclusionTexture occlusionTexture{};
		TextureInfo emissiveTexture{};
		dvec3 emissiveFactor{ base::constants::dvec3zeros };
		std::string alphaMode{ base::enumerators::alpha_mode::opaque };
		double alphaCutoff{ 0.5 };
		bool doubleSided{ false };

		// Extensions
		extensions::EXT_materials_displacement::MaterialsDisplacement extMaterialDisplacement{};
		extensions::KHR_materials_anisotropy::MaterialsAnisotropy khrMaterialAnisotropy{};
		extensions::KHR_materials_clearcoat::MaterialClearcoat khrMaterialClearcoat{};
		extensions::KHR_materials_emissive_strength::MaterialEmissiveStrength khrMaterialEmissiveStrength{};
		extensions::KHR_materials_ior::MaterialIor khrMaterialIor{};
		extensions::KHR_materials_iridescence::MaterialIridescence khrMaterialIridescence{};
		extensions::KHR_materials_sheen::MaterialSheen khrMaterialSheen{};
		extensions::KHR_materials_specular::MaterialSpecular khrMaterialSpecular{};
		extensions::KHR_materials_transmission::MaterialTransmission khrMaterialTransmission{};
		extensions::KHR_materials_unlit::MaterialUnlit khrMaterialUnlit{};
		extensions::KHR_materials_volume::MaterialVolume khrMaterialVolume{};

		bool operator==(const Material& other) const
		{
			return
				this->name == other.name && this->pbrMetallicRoughness == other.pbrMetallicRoughness &&
				this->normalTexture == other.normalTexture && this->occlusionTexture == other.occlusionTexture &&
				this->emissiveTexture == other.emissiveTexture && compare_dvec3(this->emissiveFactor, other.emissiveFactor) &&
				this->alphaMode == other.alphaMode && compare_double(this->alphaCutoff, other.alphaCutoff) &&
				this->extMaterialDisplacement == other.extMaterialDisplacement && this->doubleSided == other.doubleSided && 
				this->khrMaterialAnisotropy == other.khrMaterialAnisotropy && this->khrMaterialClearcoat == other.khrMaterialClearcoat && 
				this->khrMaterialEmissiveStrength == other.khrMaterialEmissiveStrength && this->khrMaterialIor == other.khrMaterialIor && 
				this->khrMaterialIridescence == other.khrMaterialIridescence && this->khrMaterialSheen == other.khrMaterialSheen && 
				this->khrMaterialSpecular == other.khrMaterialSpecular && this->khrMaterialTransmission == other.khrMaterialTransmission && 
				this->khrMaterialUnlit == other.khrMaterialUnlit && this->khrMaterialVolume == other.khrMaterialVolume && 
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
	struct Primitive : public Default
	{
		std::unordered_map<std::string, int32_t> attributes; // (required)
		std::vector<std::unordered_map<std::string, int32_t>> targets;

		int32_t material{ base::constants::not_set };
		int32_t indices{ base::constants::not_set };
		int32_t mode{ base::constants::not_set };

		// Extensions
		extensions::EXT_lights_shadows::CastShadows extLightShadows{};
		extensions::KHR_draco_mesh_compression::DracoMeshCompression khrDracoMeshCompression{};
		extensions::KHR_materials_variants::MaretialVariant khrMaterialVariants{};

		bool operator==(const Primitive& other) const
		{
			return
				this->attributes == other.attributes && this->targets == other.targets &&
				this->material == other.material && this->indices == other.indices &&
				this->mode == other.mode && this->extLightShadows == other.extLightShadows &&
				this->khrDracoMeshCompression == other.khrDracoMeshCompression && this->khrMaterialVariants == other.khrMaterialVariants &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
	struct Mesh : public Default
	{
		std::string name;
		std::vector<Primitive> primitives; // (required)
		std::vector<double> weights;

		extensions::EXT_lights_shadows::CastShadows extLightShadows{};

		bool operator==(const Mesh& other) const
		{
			return
				this->name == other.name && this->primitives == other.primitives &&
				this->extLightShadows == other.extLightShadows && compare_dvector(this->weights, other.weights) && 
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
	struct Node : public  Default
	{
		std::string name;

		int32_t camera{ base::constants::not_set };
		int32_t skin{ base::constants::not_set };
		int32_t mesh{ base::constants::not_set };

		std::vector<int32_t> children;
		dvec3 translation{ base::constants::dvec3zeros }; // (required)
		dvec4 rotation{ base::constants::dvec4quat_default }; // (required)
		dvec3 scale{ base::constants::dvec3ones }; // (required)
		dmat4 matrix{ base::constants::dmat4identity }; // (required)
		std::vector<double> weights{};

		// Extensions
		extensions::KHR_audio::AudioNode khrAudio{};
		extensions::KHR_lights_punctual::LightsPunctualNode khrLightsPunctual{};

		bool operator==(const Node& other) const
		{
			return
				this->name == other.name && this->camera == other.camera &&
				this->skin == other.skin && this->mesh == other.mesh &&
				this->children == other.children && compare_dvec3(this->translation, other.translation) &&
				compare_dvec4(this->rotation, other.rotation) && compare_dvec3(this->scale, other.scale) &&
				compare_dmat4(this->matrix, other.matrix) && compare_dvector(this->weights, other.weights) &&
				this->khrLightsPunctual == other.khrLightsPunctual && this->khrAudio == other.khrAudio &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
	struct Sampler : public Default
	{
		std::string name{};
		int32_t minFilter{ base::constants::not_set };
		int32_t magFilter{ base::constants::not_set };
		int32_t wrapS{ base::enumerators::texture_wrap::repeat };
		int32_t wrapT{ base::enumerators::texture_wrap::repeat };

		bool operator==(const Sampler& other) const
		{
			return
				this->name == other.name && this->minFilter == other.minFilter &&
				this->magFilter == other.magFilter && this->wrapS == other.wrapS &&
				this->wrapT == other.wrapT && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/scene.schema.json
	struct Scene : public Default
	{
		std::string name;
		std::vector<int32_t> nodes;

		extensions::KHR_audio::AudioScene khrAudio{};

		bool operator==(const Scene& other) const
		{
			return
				this->name == other.name && this->nodes == other.nodes &&
				this->khrAudio == other.khrAudio && compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/skin.schema.json
	struct Skin : public Default
	{
		std::string name{};
		int32_t inverseBindMatrices{ base::constants::not_set };
		int32_t skeleton{ base::constants::not_set };
		std::vector<int32_t> joints{}; // (required)

		bool operator==(const Skin& other) const
		{
			return
				this->name == other.name && this->inverseBindMatrices == other.inverseBindMatrices &&
				this->skeleton == other.skeleton && this->joints == other.joints &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/texture.schema.json
	struct Texture : public Default
	{
		std::string name;

		int32_t sampler{ base::constants::not_set };
		int32_t source{ base::constants::not_set };

		// Extensions
		extensions::KHR_texture_basisu::TextureBasisU khrTextureBasisU{};

		bool operator==(const Texture& other) const
		{
			return
				this->name == other.name && this->sampler == other.sampler &&
				this->source == other.source && this->khrTextureBasisU == other.khrTextureBasisU &&
				compareExtensionsAndExtras(other);
		}
	};

	// https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
	struct FGltf : public Default
	{
		std::vector<Accessor> accessors{};
		std::vector<Animation> animations{};
		Asset asset{};
		std::vector<Buffer> buffers{};
		std::vector<BufferView> bufferViews{};
		std::vector<Camera> cameras{};
		std::vector<Image> images{};
		std::vector<Material> materials{};
		std::vector<Mesh> meshes{};
		std::vector<Node> nodes{};
		std::vector<Sampler> samplers{};
		std::vector<Scene> scenes{};
		std::vector<Skin> skins{};
		std::vector<Texture> textures{};

		// Extensions
		extensions::KHR_audio::Audio khrAudio{};
		extensions::KHR_lights_punctual::LightsPunctual khrLightsPunctual{};
		extensions::KHR_materials_variants::MaterialVariants khrMaterialVariants{};

		int32_t scene{ -1 }; // The index of the default scene.

		mutable std::unordered_set<std::string> extensionsUsed;
		mutable std::unordered_set<std::string> extensionsRequired;
	};
}
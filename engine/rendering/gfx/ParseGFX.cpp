#include "ParseGFX.h"
#include "StringGFX.h"

namespace engine
{
	namespace gfx
	{
        template<class _Ty>
        inline void parse_from(const std::string& name, const nlohmann::json& json, _Ty& type, bool required = false)
        {
            if (auto obj = json.find(name); obj != json.end())
                type = obj->get<_Ty>();
            else
                if (required)
                    throw std::runtime_error("Required argument was not found");
        }

        template<class _Ty>
        inline void serialize_to(const std::string& name, nlohmann::json& json, const _Ty& type, bool condition = true)
        {
            if (condition)
                json[name] = type;
        }

		

		// Format
		void to_json(nlohmann::json& json, const Format& type) { json = to_string(type); } 
		void from_json(const nlohmann::json& json, Format& type) { type = from_string<Format>(json.get<std::string>()); }


		// ShaderStageFlag
		void to_json(nlohmann::json& json, const ShaderStageFlag& type) { json = to_string(type); }
		void from_json(const nlohmann::json& json, ShaderStageFlag& type) { type = from_string<ShaderStageFlag>(json.get<std::string>()); }

		template<typename _FlagT, size_t _MaxFlags>
		std::vector<std::string> serialize_hr_flags(const utl::flags<_FlagT>& flags, const std::array<_FlagT, _MaxFlags>& all_flags)
		{
			std::vector<std::string> flags_vector{ _MaxFlags };
			for (int32_t i = 0; i < _MaxFlags; ++i)
			{
				if (utl::eq_(flags, all_flags[i]))
				{
					flags_vector[i] = to_string(all_flags[i]);
				}
			}
			
			return flags_vector;
		}

		template<typename _FlagT>
		utl::flags<_FlagT> deserialize_hr_flags(const std::vector<std::string>& flags_vector)
		{
			utl::flags<_FlagT> flags{};
			for (int32_t i = 0; i < flags_vector.size(); ++i)
			{
				flags |= from_string<_FlagT>(flags_vector[i]);
			}
			return flags;
		}

		// ShaderStageFlags human readable
		void to_json(nlohmann::json& json, const ShaderStageFlags& type)
		{
			static std::array<ShaderStageFlag, 6ull> all_flags {
				ShaderStageFlag::eVertex,
				ShaderStageFlag::eTessellationControl,
				ShaderStageFlag::eTessellationEvaluation,
				ShaderStageFlag::eGeometry,
				ShaderStageFlag::eFragment,
				ShaderStageFlag::eCompute,
			};

			json = serialize_hr_flags(type, all_flags);
		}

		void from_json(const nlohmann::json& json, ShaderStageFlags& type)
		{
			type = deserialize_hr_flags<ShaderStageFlag>(json.get<std::vector<std::string>>());
		}


		// QueueTypeFlag
		void to_json(nlohmann::json& json, const QueueTypeFlag& type)
		{

		}

		void from_json(const nlohmann::json& json, QueueTypeFlag& type)
		{

		}


		// QueueTypeFlags
		void to_json(nlohmann::json& json, const QueueTypeFlags& type)
		{

		}

		void from_json(const nlohmann::json& json, QueueTypeFlags& type)
		{

		}


		// Filter
		void to_json(nlohmann::json& json, const Filter& type)
		{

		}

		void from_json(const nlohmann::json& json, Filter& type)
		{

		}


		// SamplerAddressMode
		void to_json(nlohmann::json& json, const SamplerAddressMode& type)
		{

		}

		void from_json(const nlohmann::json& json, SamplerAddressMode& type)
		{

		}


		// CompareOp
		void to_json(nlohmann::json& json, const CompareOp& type)
		{

		}

		void from_json(const nlohmann::json& json, CompareOp& type)
		{

		}


		// PipelineType
		void to_json(nlohmann::json& json, const PipelineType& type)
		{

		}

		void from_json(const nlohmann::json& json, PipelineType& type)
		{

		}


		// PrimitiveTopology
		void to_json(nlohmann::json& json, const PrimitiveTopology& type)
		{

		}

		void from_json(const nlohmann::json& json, PrimitiveTopology& type)
		{

		}


		// SampleCount
		void to_json(nlohmann::json& json, const SampleCount& type)
		{

		}

		void from_json(const nlohmann::json& json, SampleCount& type)
		{

		}


		// FillMode
		void to_json(nlohmann::json& json, const FillMode& type)
		{

		}

		void from_json(const nlohmann::json& json, FillMode& type)
		{

		}


		// CullModeFlag
		void to_json(nlohmann::json& json, const CullModeFlag& type)
		{

		}

		void from_json(const nlohmann::json& json, CullModeFlag& type)
		{

		}


		// CullModeFlags
		void to_json(nlohmann::json& json, const CullModeFlags& type)
		{

		}

		void from_json(const nlohmann::json& json, CullModeFlags& type)
		{

		}


		// FrontFace
		void to_json(nlohmann::json& json, const FrontFace& type)
		{

		}

		void from_json(const nlohmann::json& json, FrontFace& type)
		{

		}


		// StencilOp
		void to_json(nlohmann::json& json, const StencilOp& type)
		{

		}

		void from_json(const nlohmann::json& json, StencilOp& type)
		{

		}


		// BlendFactor
		void to_json(nlohmann::json& json, const BlendFactor& type)
		{

		}

		void from_json(const nlohmann::json& json, BlendFactor& type)
		{

		}


		// BlendOp
		void to_json(nlohmann::json& json, const BlendOp& type)
		{

		}

		void from_json(const nlohmann::json& json, BlendOp& type)
		{

		}


		// ColorMaskFlag
		void to_json(nlohmann::json& json, const ColorMaskFlag& type)
		{

		}

		void from_json(const nlohmann::json& json, ColorMaskFlag& type)
		{

		}


		// ColorMaskFlags
		void to_json(nlohmann::json& json, const ColorMaskFlags& type)
		{

		}

		void from_json(const nlohmann::json& json, ColorMaskFlags& type)
		{

		}


		// DescriptorType
		void to_json(nlohmann::json& json, const DescriptorType& type)
		{

		}

		void from_json(const nlohmann::json& json, DescriptorType& type)
		{

		}

	}
}
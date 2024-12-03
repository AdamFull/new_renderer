#include "Parse.h"

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
		void to_json(nlohmann::json& json, const Format& type)
		{

		}

		void from_json(const nlohmann::json& json, Format& type)
		{

		}


		// ShaderStageFlag
		void to_json(nlohmann::json& json, const ShaderStageFlag& type)
		{

		}

		void from_json(const nlohmann::json& json, ShaderStageFlag& type)
		{

		}


		// ShaderStageFlags
		void to_json(nlohmann::json& json, const ShaderStageFlags& type)
		{

		}

		void from_json(const nlohmann::json& json, ShaderStageFlags& type)
		{

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
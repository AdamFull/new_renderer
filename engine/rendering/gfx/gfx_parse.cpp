#include "gfx_parse.h"
#include "gfx_string.h"

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


		template<typename _FlagT>
		std::vector<std::string> serialize_hr_flags(const utl::flags<_FlagT>& flags)
		{
			std::vector<std::string> flags_vector{ utl::enum_metadata<_FlagT>.size() };
			for (int32_t i = 0; i < utl::enum_metadata<_FlagT>.size(); ++i)
			{
				if (utl::eq_(flags, utl::enum_metadata<_FlagT>[i].first))
				{
					flags_vector[i] = utl::enum_metadata<_FlagT>[i].second;
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
				flags |= utl::enum_from_string<_FlagT>(flags_vector[i].c_str());
			}
			return flags;
		}

		// ShaderStageFlags human readable
		void to_json(nlohmann::json& json, const ShaderStageFlags& type) { json = serialize_hr_flags(type); }
		void from_json(const nlohmann::json& json, ShaderStageFlags& type) { type = deserialize_hr_flags<ShaderStageFlag>(json.get<std::vector<std::string>>()); }


		// QueueTypeFlags
		void to_json(nlohmann::json& json, const QueueTypeFlags& type) { json = serialize_hr_flags(type); }
		void from_json(const nlohmann::json& json, QueueTypeFlags& type) { type = deserialize_hr_flags<QueueTypeFlag>(json.get<std::vector<std::string>>()); }


		// CullModeFlags
		void to_json(nlohmann::json& json, const CullModeFlags& type) { json = serialize_hr_flags(type); }
		void from_json(const nlohmann::json& json, CullModeFlags& type) { type = deserialize_hr_flags<CullModeFlag>(json.get<std::vector<std::string>>()); }


		// ColorMaskFlags
		void to_json(nlohmann::json& json, const ColorMaskFlags& type) { json = serialize_hr_flags(type); }
		void from_json(const nlohmann::json& json, ColorMaskFlags& type) { type = deserialize_hr_flags<ColorMaskFlag>(json.get<std::vector<std::string>>()); }

		
		void to_json(nlohmann::json& json, const RasterizationState& type)
		{
			serialize_to("depth_clamp", json, type.depth_clamp);
			serialize_to("discard_enable", json, type.discard_enable);
			serialize_to("fill_mode", json, type.fill_mode);
			serialize_to("cull_mode", json, type.cull_mode);
			serialize_to("front_face", json, type.front_face);
		}

		void from_json(const nlohmann::json& json, RasterizationState& type)
		{
			parse_from("depth_clamp", json, type.depth_clamp);
			parse_from("discard_enable", json, type.discard_enable);
			parse_from("fill_mode", json, type.fill_mode);
			parse_from("cull_mode", json, type.cull_mode);
			parse_from("front_face", json, type.front_face);
		}


		void to_json(nlohmann::json& json, const MultisampleState& type)
		{
			serialize_to("sample_count", json, type.sample_count);
			serialize_to("alpha_to_coverage", json, type.alpha_to_coverage);
			serialize_to("alpha_to_one", json, type.alpha_to_one);
		}

		void from_json(const nlohmann::json& json, MultisampleState& type)
		{
			parse_from("sample_count", json, type.sample_count);
			parse_from("alpha_to_coverage", json, type.alpha_to_coverage);
			parse_from("alpha_to_one", json, type.alpha_to_one);
		}


		void to_json(nlohmann::json& json, const DepthStencilState& type)
		{
			serialize_to("depth_test", json, type.depth_test);
			serialize_to("depth_write", json, type.depth_write);
			serialize_to("depth_compare_op", json, type.depth_compare_op);

			serialize_to("stencil_test", json, type.stencil_test);
			serialize_to("stencil_front_compare_op", json, type.stencil_front_compare_op);
			serialize_to("front_fail", json, type.front_fail);
			serialize_to("depth_front_fail", json, type.depth_front_fail);
			serialize_to("front_pass", json, type.front_pass);

			serialize_to("stencil_back_compare_op", json, type.stencil_back_compare_op);
			serialize_to("back_fail", json, type.back_fail);
			serialize_to("depth_back_fail", json, type.depth_back_fail);
			serialize_to("back_pass", json, type.back_pass);
		}

		void from_json(const nlohmann::json& json, DepthStencilState& type)
		{
			parse_from("depth_test", json, type.depth_test);
			parse_from("depth_write", json, type.depth_write);
			parse_from("depth_compare_op", json, type.depth_compare_op);

			parse_from("stencil_test", json, type.stencil_test);
			parse_from("stencil_front_compare_op", json, type.stencil_front_compare_op);
			parse_from("front_fail", json, type.front_fail);
			parse_from("depth_front_fail", json, type.depth_front_fail);
			parse_from("front_pass", json, type.front_pass);

			parse_from("stencil_back_compare_op", json, type.stencil_back_compare_op);
			parse_from("back_fail", json, type.back_fail);
			parse_from("depth_back_fail", json, type.depth_back_fail);
			parse_from("back_pass", json, type.back_pass);
		}


		void to_json(nlohmann::json& json, const ColorBlendAttachment& type)
		{
			serialize_to("enable", json, type.enable);
			serialize_to("src_color", json, type.src_color);
			serialize_to("dst_color", json, type.dst_color);
			serialize_to("color_blend_op", json, type.color_blend_op);
			serialize_to("src_alpha", json, type.src_alpha);
			serialize_to("dst_alpha", json, type.dst_alpha);
			serialize_to("alpha_blend_op", json, type.alpha_blend_op);
			serialize_to("color_mask", json, type.color_mask);
		}

		void from_json(const nlohmann::json& json, ColorBlendAttachment& type)
		{
			parse_from("enable", json, type.enable);
			parse_from("src_color", json, type.src_color);
			parse_from("dst_color", json, type.dst_color);
			parse_from("color_blend_op", json, type.color_blend_op);
			parse_from("src_alpha", json, type.src_alpha);
			parse_from("dst_alpha", json, type.dst_alpha);
			parse_from("alpha_blend_op", json, type.alpha_blend_op);
			parse_from("color_mask", json, type.color_mask);
		}


		void to_json(nlohmann::json& json, const ColorBlendingState& type)
		{
			serialize_to("attachments", json, type.attachments);
			serialize_to("attachment_count", json, type.attachment_count);
		}

		void from_json(const nlohmann::json& json, ColorBlendingState& type)
		{
			parse_from("attachments", json, type.attachments);
			parse_from("attachment_count", json, type.attachment_count);
		}
	}
}
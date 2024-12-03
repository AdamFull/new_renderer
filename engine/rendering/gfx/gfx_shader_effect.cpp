#include "gfx_shader_effect.h"

#include "gfx_parse.h"

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

namespace engine
{
	namespace gfx
	{
		namespace shfx
		{
			void to_json(nlohmann::json& json, const ShaderEffectCompileSourceDesc& type)
			{
				serialize_to("source", json, type.source);
				serialize_to("type", json, type.type);
				serialize_to("definitions", json, type.definitions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSourceDesc& type)
			{
				parse_from("source", json, type.source, true);
				parse_from("type", json, type.type, true);
				parse_from("definitions", json, type.definitions);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompileSamplerDesc& type)
			{
				serialize_to("binding", json, type.binding);

				//serialize_to("address_u", json, type.address_u);
				//serialize_to("address_v", json, type.address_v);
				//serialize_to("address_w", json, type.address_w);
				//
				//serialize_to("min_filter", json, type.min_filter);
				//serialize_to("mag_filter", json, type.mag_filter);
				//serialize_to("mip_filter", json, type.mip_filter);
				//
				//serialize_to("mip_lod_bias", json, type.mip_lod_bias);
				//serialize_to("min_lod", json, type.min_lod);
				//serialize_to("max_lod", json, type.max_lod);
				//serialize_to("max_anisotropy", json, type.max_anisotropy);
				//
				//serialize_to("compare_op", json, type.compare_op);

			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSamplerDesc& type)
			{
				parse_from("binding", json, type.binding, true);
				//parse_from("address_u", json, type.address_u);
				//parse_from("address_v", json, type.address_v);
				//parse_from("address_w", json, type.address_w);
				//
				//parse_from("min_filter", json, type.min_filter);
				//parse_from("mag_filter", json, type.mag_filter);
				//parse_from("mip_filter", json, type.mip_filter);
				//
				//parse_from("mip_lod_bias", json, type.mip_lod_bias);
				//parse_from("min_lod", json, type.min_lod);
				//parse_from("max_lod", json, type.max_lod);
				//parse_from("max_anisotropy", json, type.max_anisotropy);
				//
				//parse_from("compare_op", json, type.compare_op);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilePipelineState& type)
			{
				serialize_to("rasterization_state", json, type.rasterization_state);
				serialize_to("multisample_state", json, type.multisample_state);
				serialize_to("depth_stencil_state", json, type.depth_stencil_state);
				serialize_to("color_blending_state", json, type.color_blending_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilePipelineState& type)
			{
				parse_from("rasterization_state", json, type.rasterization_state);
				parse_from("multisample_state", json, type.multisample_state);
				parse_from("depth_stencil_state", json, type.depth_stencil_state);
				parse_from("color_blending_state", json, type.color_blending_state);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompileDesc& type)
			{
				serialize_to("name", json, type.name);
				serialize_to("shaders", json, type.shaders);
				serialize_to("samplers", json, type.samplers);
				serialize_to("pipeline_state", json, type.pipeline_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileDesc& type)
			{
				parse_from("name", json, type.name);
				parse_from("shaders", json, type.shaders);
				parse_from("samplers", json, type.samplers);
				parse_from("pipeline_state", json, type.pipeline_state);
			}


			void to_json(nlohmann::json& json, const ShaderEffectDesc& type)
			{

			}

			void from_json(const nlohmann::json& json, ShaderEffectDesc& type)
			{

			}
		}
	}
}
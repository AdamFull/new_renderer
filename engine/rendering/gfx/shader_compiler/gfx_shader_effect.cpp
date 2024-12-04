#include "gfx_shader_effect.h"

#include <serialization.hpp>

namespace engine
{
	namespace gfx
	{
		namespace shfx
		{
			void to_json(nlohmann::json& json, const ShaderEffectCompileSourceDesc& type)
			{
				utl::serialize_to("source", json, type.source);
				utl::serialize_to("type", json, type.type);
				utl::serialize_to("definitions", json, type.definitions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSourceDesc& type)
			{
				utl::parse_from("source", json, type.source, true);
				utl::parse_from("type", json, type.type, true);
				utl::parse_from("definitions", json, type.definitions);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompileSamplerDesc& type)
			{
				utl::serialize_to("binding", json, type.binding);

				utl::serialize_to("address_u", json, type.sampler_create_info.address_u);
				utl::serialize_to("address_v", json, type.sampler_create_info.address_v);
				utl::serialize_to("address_w", json, type.sampler_create_info.address_w);
				
				utl::serialize_to("min_filter", json, type.sampler_create_info.min_filter);
				utl::serialize_to("mag_filter", json, type.sampler_create_info.mag_filter);
				utl::serialize_to("mip_filter", json, type.sampler_create_info.mip_map_mode);
				
				utl::serialize_to("mip_lod_bias", json, type.sampler_create_info.mip_lod_bias);
				utl::serialize_to("min_lod", json, type.sampler_create_info.min_lod);
				utl::serialize_to("max_lod", json, type.sampler_create_info.max_lod);
				utl::serialize_to("max_anisotropy", json, type.sampler_create_info.max_anisotropy);
				
				utl::serialize_to("compare_op", json, type.sampler_create_info.depth_compare);

			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSamplerDesc& type)
			{
				utl::parse_from("binding", json, type.binding, true);
				
				utl::parse_from("address_u", json, type.sampler_create_info.address_u);
				utl::parse_from("address_v", json, type.sampler_create_info.address_v);
				utl::parse_from("address_w", json, type.sampler_create_info.address_w);

				utl::parse_from("min_filter", json, type.sampler_create_info.min_filter);
				utl::parse_from("mag_filter", json, type.sampler_create_info.mag_filter);
				utl::parse_from("mip_filter", json, type.sampler_create_info.mip_map_mode);

				utl::parse_from("mip_lod_bias", json, type.sampler_create_info.mip_lod_bias);
				utl::parse_from("min_lod", json, type.sampler_create_info.min_lod);
				utl::parse_from("max_lod", json, type.sampler_create_info.max_lod);
				utl::parse_from("max_anisotropy", json, type.sampler_create_info.max_anisotropy);

				utl::parse_from("compare_op", json, type.sampler_create_info.depth_compare);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilePipelineState& type)
			{
				utl::serialize_to("rasterization_state", json, type.rasterization_state);
				utl::serialize_to("multisample_state", json, type.multisample_state);
				utl::serialize_to("depth_stencil_state", json, type.depth_stencil_state);
				utl::serialize_to("color_blending_state", json, type.color_blending_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilePipelineState& type)
			{
				utl::parse_from("rasterization_state", json, type.rasterization_state);
				utl::parse_from("multisample_state", json, type.multisample_state);
				utl::parse_from("depth_stencil_state", json, type.depth_stencil_state);
				utl::parse_from("color_blending_state", json, type.color_blending_state);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompileDesc& type)
			{
				utl::serialize_to("name", json, type.name);
				utl::serialize_to("shaders", json, type.shaders);
				utl::serialize_to("samplers", json, type.samplers);
				utl::serialize_to("pipeline_state", json, type.pipeline_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileDesc& type)
			{
				utl::parse_from("name", json, type.name);
				utl::parse_from("shaders", json, type.shaders);
				utl::parse_from("samplers", json, type.samplers);
				utl::parse_from("pipeline_state", json, type.pipeline_state);
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
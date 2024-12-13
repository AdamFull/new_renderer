#include "gfx_structures.h"
#include <serialization.hpp>

namespace edge
{
	namespace gfx
	{
		void to_json(nlohmann::json& json, const VertexAttributeDescription& type)
		{
			utl::serialize_to("location", json, type.location);
			utl::serialize_to("binding", json, type.binding);
			//utl::serialize_to("format", json, type.format);
			utl::serialize_to("offset", json, type.offset);
		}

		void from_json(const nlohmann::json& json, VertexAttributeDescription& type)
		{
			utl::parse_from("location", json, type.location);
			utl::parse_from("binding", json, type.binding);
			//utl::parse_from("format", json, type.format);
			utl::parse_from("offset", json, type.offset);
		}


		void to_json(nlohmann::json& json, const VertexBindingDescription& type)
		{
			utl::serialize_to("binding", json, type.binding);
			utl::serialize_to("stride", json, type.stride);
		}

		void from_json(const nlohmann::json& json, VertexBindingDescription& type)
		{
			utl::parse_from("binding", json, type.binding);
			utl::parse_from("stride", json, type.stride);
		}


		void to_json(nlohmann::json& json, const VertexLayout& type)
		{
			utl::serialize_to("bindings", json, type.bindings);
			utl::serialize_to("binding_count", json, type.binding_count);
			utl::serialize_to("attributes", json, type.attributes);
			utl::serialize_to("attribute_count", json, type.attribute_count);
		}

		void from_json(const nlohmann::json& json, VertexLayout& type)
		{
			utl::parse_from("bindings", json, type.bindings);
			utl::parse_from("binding_count", json, type.binding_count);
			utl::parse_from("attributes", json, type.attributes);
			utl::parse_from("attribute_count", json, type.attribute_count);
		}


		void to_json(nlohmann::json& json, const RasterizationState& type)
		{
			utl::serialize_to("depth_clamp", json, type.depth_clamp);
			utl::serialize_to("discard_enable", json, type.discard_enable);
			utl::serialize_to("fill_mode", json, type.fill_mode);
			utl::serialize_to("cull_mode", json, type.cull_mode);
			utl::serialize_to("front_face", json, type.front_face);
		}

		void from_json(const nlohmann::json& json, RasterizationState& type)
		{
			utl::parse_from("depth_clamp", json, type.depth_clamp);
			utl::parse_from("discard_enable", json, type.discard_enable);
			utl::parse_from("fill_mode", json, type.fill_mode);
			utl::parse_from("cull_mode", json, type.cull_mode);
			utl::parse_from("front_face", json, type.front_face);
		}


		void to_json(nlohmann::json& json, const MultisampleState& type)
		{
			utl::serialize_to("sample_count", json, type.sample_count);
			utl::serialize_to("alpha_to_coverage", json, type.alpha_to_coverage);
			utl::serialize_to("alpha_to_one", json, type.alpha_to_one);
		}

		void from_json(const nlohmann::json& json, MultisampleState& type)
		{
			utl::parse_from("sample_count", json, type.sample_count);
			utl::parse_from("alpha_to_coverage", json, type.alpha_to_coverage);
			utl::parse_from("alpha_to_one", json, type.alpha_to_one);
		}


		void to_json(nlohmann::json& json, const DepthStencilState& type)
		{
			utl::serialize_to("depth_test", json, type.depth_test);
			utl::serialize_to("depth_write", json, type.depth_write);
			utl::serialize_to("depth_compare_op", json, type.depth_compare_op);

			utl::serialize_to("stencil_test", json, type.stencil_test);
			utl::serialize_to("stencil_front_compare_op", json, type.stencil_front_compare_op);
			utl::serialize_to("front_fail", json, type.front_fail);
			utl::serialize_to("depth_front_fail", json, type.depth_front_fail);
			utl::serialize_to("front_pass", json, type.front_pass);

			utl::serialize_to("stencil_back_compare_op", json, type.stencil_back_compare_op);
			utl::serialize_to("back_fail", json, type.back_fail);
			utl::serialize_to("depth_back_fail", json, type.depth_back_fail);
			utl::serialize_to("back_pass", json, type.back_pass);
		}

		void from_json(const nlohmann::json& json, DepthStencilState& type)
		{
			utl::parse_from("depth_test", json, type.depth_test);
			utl::parse_from("depth_write", json, type.depth_write);
			utl::parse_from("depth_compare_op", json, type.depth_compare_op);

			utl::parse_from("stencil_test", json, type.stencil_test);
			utl::parse_from("stencil_front_compare_op", json, type.stencil_front_compare_op);
			utl::parse_from("front_fail", json, type.front_fail);
			utl::parse_from("depth_front_fail", json, type.depth_front_fail);
			utl::parse_from("front_pass", json, type.front_pass);

			utl::parse_from("stencil_back_compare_op", json, type.stencil_back_compare_op);
			utl::parse_from("back_fail", json, type.back_fail);
			utl::parse_from("depth_back_fail", json, type.depth_back_fail);
			utl::parse_from("back_pass", json, type.back_pass);
		}


		void to_json(nlohmann::json& json, const ColorBlendAttachment& type)
		{
			utl::serialize_to("enable", json, type.enable);
			utl::serialize_to("src_color", json, type.src_color);
			utl::serialize_to("dst_color", json, type.dst_color);
			utl::serialize_to("color_blend_op", json, type.color_blend_op);
			utl::serialize_to("src_alpha", json, type.src_alpha);
			utl::serialize_to("dst_alpha", json, type.dst_alpha);
			utl::serialize_to("alpha_blend_op", json, type.alpha_blend_op);
			utl::serialize_to("color_mask", json, type.color_mask);
		}

		void from_json(const nlohmann::json& json, ColorBlendAttachment& type)
		{
			utl::parse_from("enable", json, type.enable);
			utl::parse_from("src_color", json, type.src_color);
			utl::parse_from("dst_color", json, type.dst_color);
			utl::parse_from("color_blend_op", json, type.color_blend_op);
			utl::parse_from("src_alpha", json, type.src_alpha);
			utl::parse_from("dst_alpha", json, type.dst_alpha);
			utl::parse_from("alpha_blend_op", json, type.alpha_blend_op);
			utl::parse_from("color_mask", json, type.color_mask);
		}


		void to_json(nlohmann::json& json, const ColorBlendingState& type)
		{
			utl::serialize_to("attachments", json, type.attachments);
			utl::serialize_to("attachment_count", json, type.attachment_count);
		}

		void from_json(const nlohmann::json& json, ColorBlendingState& type)
		{
			utl::parse_from("attachments", json, type.attachments);
			utl::parse_from("attachment_count", json, type.attachment_count);
		}


		void to_json(nlohmann::json& json, const PipelineState& type)
		{
			utl::serialize_to("vertex_layout", json, type.vertex_layout);
			utl::serialize_to("rasterization_state", json, type.rasterization_state);
			utl::serialize_to("multisample_state", json, type.multisample_state);
			utl::serialize_to("depth_stencil_state", json, type.depth_stencil_state);
			utl::serialize_to("color_blending_state", json, type.color_blending_state);
			utl::serialize_to("primitive_topology", json, type.primitive_topology);
		}

		void from_json(const nlohmann::json& json, PipelineState& type)
		{
			utl::parse_from("vertex_layout", json, type.vertex_layout);
			utl::parse_from("rasterization_state", json, type.rasterization_state);
			utl::parse_from("multisample_state", json, type.multisample_state);
			utl::parse_from("depth_stencil_state", json, type.depth_stencil_state);
			utl::parse_from("color_blending_state", json, type.color_blending_state);
			utl::parse_from("primitive_topology", json, type.primitive_topology);
		}


		void to_json(nlohmann::json& json, const BufferCreateInfo& type)
		{
			utl::serialize_to("usage", json, type.usage);
			utl::serialize_to("size", json, type.size);
			utl::serialize_to("count", json, type.count);
		}

		void from_json(const nlohmann::json& json, BufferCreateInfo& type)
		{
			utl::parse_from("usage", json, type.usage, true);
			utl::parse_from("size", json, type.size, true);
			utl::parse_from("count", json, type.count);
		}


		void to_json(nlohmann::json& json, const TextureCreateInfo& type)
		{
			utl::serialize_to("usage", json, type.usage);

			utl::serialize_to("width", json, type.width);
			utl::serialize_to("height", json, type.height);
			utl::serialize_to("depth", json, type.depth);

			utl::serialize_to("layers", json, type.layers);
			utl::serialize_to("levels", json, type.levels);

			//utl::serialize_to("format", json, type.format);
		}

		void from_json(const nlohmann::json& json, TextureCreateInfo& type)
		{
			utl::parse_from("usage", json, type.usage);

			utl::parse_from("width", json, type.width);
			utl::parse_from("height", json, type.height);
			utl::parse_from("depth", json, type.depth);

			utl::parse_from("layers", json, type.layers);
			utl::parse_from("levels", json, type.levels);

			//utl::parse_from("format", json, type.format);
		}


		void to_json(nlohmann::json& json, const SamplerCreateInfo& type)
		{
			utl::serialize_to("min_filter", json, type.min_filter);
			utl::serialize_to("mag_filter", json, type.mag_filter);
			utl::serialize_to("mip_map_mode", json, type.mip_map_mode);

			utl::serialize_to("address_u", json, type.address_u);
			utl::serialize_to("address_v", json, type.address_v);
			utl::serialize_to("address_w", json, type.address_w);

			utl::serialize_to("mip_lod_bias", json, type.mip_lod_bias);
			utl::serialize_to("min_lod", json, type.min_lod);
			utl::serialize_to("max_lod", json, type.max_lod);
			utl::serialize_to("max_anisotropy", json, type.max_anisotropy);

			utl::serialize_to("depth_compare", json, type.depth_compare);
		}

		void from_json(const nlohmann::json& json, SamplerCreateInfo& type)
		{
			utl::parse_from("min_filter", json, type.min_filter);
			utl::parse_from("mag_filter", json, type.mag_filter);
			utl::parse_from("mip_map_mode", json, type.mip_map_mode);

			utl::parse_from("address_u", json, type.address_u);
			utl::parse_from("address_v", json, type.address_v);
			utl::parse_from("address_w", json, type.address_w);

			utl::parse_from("mip_lod_bias", json, type.mip_lod_bias);
			utl::parse_from("min_lod", json, type.min_lod);
			utl::parse_from("max_lod", json, type.max_lod);
			utl::parse_from("max_anisotropy", json, type.max_anisotropy);

			utl::parse_from("depth_compare", json, type.depth_compare);
		}
	}
}
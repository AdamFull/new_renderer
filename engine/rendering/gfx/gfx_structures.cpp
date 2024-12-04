#include "gfx_structures.h"

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


		void to_json(nlohmann::json& json, const VertexAttributeDescription& type)
		{
			serialize_to("location", json, type.location);
			serialize_to("binding", json, type.binding);
			//serialize_to("format", json, type.format);
			serialize_to("offset", json, type.offset);
		}

		void from_json(const nlohmann::json& json, VertexAttributeDescription& type)
		{
			parse_from("location", json, type.location);
			parse_from("binding", json, type.binding);
			//parse_from("format", json, type.format);
			parse_from("offset", json, type.offset);
		}


		void to_json(nlohmann::json& json, const VertexBindingDescription& type)
		{
			serialize_to("binding", json, type.binding);
			serialize_to("stride", json, type.stride);
		}

		void from_json(const nlohmann::json& json, VertexBindingDescription& type)
		{
			parse_from("binding", json, type.binding);
			parse_from("stride", json, type.stride);
		}


		void to_json(nlohmann::json& json, const VertexLayout& type)
		{
			serialize_to("bindings", json, type.bindings);
			serialize_to("binding_count", json, type.binding_count);
			serialize_to("attributes", json, type.attributes);
			serialize_to("attribute_count", json, type.attribute_count);
		}

		void from_json(const nlohmann::json& json, VertexLayout& type)
		{
			parse_from("bindings", json, type.bindings);
			parse_from("binding_count", json, type.binding_count);
			parse_from("attributes", json, type.attributes);
			parse_from("attribute_count", json, type.attribute_count);
		}


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


		void to_json(nlohmann::json& json, const PipelineState& type)
		{
			serialize_to("vertex_layout", json, type.vertex_layout);
			serialize_to("rasterization_state", json, type.rasterization_state);
			serialize_to("multisample_state", json, type.multisample_state);
			serialize_to("depth_stencil_state", json, type.depth_stencil_state);
			serialize_to("color_blending_state", json, type.color_blending_state);
			serialize_to("primitive_topology", json, type.primitive_topology);
		}

		void from_json(const nlohmann::json& json, PipelineState& type)
		{
			parse_from("vertex_layout", json, type.vertex_layout);
			parse_from("rasterization_state", json, type.rasterization_state);
			parse_from("multisample_state", json, type.multisample_state);
			parse_from("depth_stencil_state", json, type.depth_stencil_state);
			parse_from("color_blending_state", json, type.color_blending_state);
			parse_from("primitive_topology", json, type.primitive_topology);
		}


		void to_json(nlohmann::json& json, const BufferCreateInfo& type)
		{
			serialize_to("usage", json, type.usage);
			serialize_to("size", json, type.size);
			serialize_to("count", json, type.count);
		}

		void from_json(const nlohmann::json& json, BufferCreateInfo& type)
		{
			parse_from("usage", json, type.usage, true);
			parse_from("size", json, type.size, true);
			parse_from("count", json, type.count);
		}


		void to_json(nlohmann::json& json, const TextureCreateInfo& type)
		{
			serialize_to("usage", json, type.usage);

			serialize_to("width", json, type.width);
			serialize_to("height", json, type.height);
			serialize_to("depth", json, type.depth);

			serialize_to("layers", json, type.layers);
			serialize_to("levels", json, type.levels);

			//serialize_to("format", json, type.format);
		}

		void from_json(const nlohmann::json& json, TextureCreateInfo& type)
		{
			parse_from("usage", json, type.usage);

			parse_from("width", json, type.width);
			parse_from("height", json, type.height);
			parse_from("depth", json, type.depth);

			parse_from("layers", json, type.layers);
			parse_from("levels", json, type.levels);

			//parse_from("format", json, type.format);
		}


		void to_json(nlohmann::json& json, const SamplerCreateInfo& type)
		{
			serialize_to("min_filter", json, type.min_filter);
			serialize_to("mag_filter", json, type.mag_filter);
			serialize_to("mip_map_mode", json, type.mip_map_mode);

			serialize_to("address_u", json, type.address_u);
			serialize_to("address_v", json, type.address_v);
			serialize_to("address_w", json, type.address_w);

			serialize_to("mip_lod_bias", json, type.mip_lod_bias);
			serialize_to("min_lod", json, type.min_lod);
			serialize_to("max_lod", json, type.max_lod);
			serialize_to("max_anisotropy", json, type.max_anisotropy);

			serialize_to("depth_compare", json, type.depth_compare);
		}

		void from_json(const nlohmann::json& json, SamplerCreateInfo& type)
		{
			parse_from("min_filter", json, type.min_filter);
			parse_from("mag_filter", json, type.mag_filter);
			parse_from("mip_map_mode", json, type.mip_map_mode);

			parse_from("address_u", json, type.address_u);
			parse_from("address_v", json, type.address_v);
			parse_from("address_w", json, type.address_w);

			parse_from("mip_lod_bias", json, type.mip_lod_bias);
			parse_from("min_lod", json, type.min_lod);
			parse_from("max_lod", json, type.max_lod);
			parse_from("max_anisotropy", json, type.max_anisotropy);

			parse_from("depth_compare", json, type.depth_compare);
		}
	}
}
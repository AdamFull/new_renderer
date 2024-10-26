#include <Context.h>

#include <vulkan/VkContext.h>
#include <hash.hpp>

namespace engine
{
	namespace gfx
	{
		uint32_t Hash(const VertexAttributeDescription& desc, uint32_t hash)
		{
			hash = utl::murmur_hash(desc.location, hash);
			hash = utl::murmur_hash(desc.binding, hash);
			hash = utl::murmur_hash(utl::enum_cast(desc.format), hash);
			hash = utl::murmur_hash(desc.offset, hash);

			return hash;
		}

		uint32_t Hash(const VertexBindingDescription& desc, uint32_t hash)
		{
			hash = utl::murmur_hash(desc.binding, hash);
			hash = utl::murmur_hash(desc.stride, hash);

			return hash;
		}

		uint32_t Hash(const VertexLayout& layout, uint32_t hash)
		{
			for (int32_t i = 0; i < layout.attribute_count; ++i)
			{
				hash = Hash(layout.attributes[i], hash);
			}

			for (int32_t i = 0; i < layout.binding_count; ++i)
			{
				hash = Hash(layout.bindings[i], hash);
			}

			return hash;
		}

		uint32_t Hash(const RasterizationState& state, uint32_t hash)
		{
			hash = utl::murmur_hash(state.depth_clamp, hash);
			hash = utl::murmur_hash(state.discard_enable, hash);
			hash = utl::murmur_hash(utl::enum_cast(state.fill_mode), hash);
			//hash = utl::murmur_hash(static_cast<uint32_t>(state.cull_mode), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.front_face), hash);

			return hash;
		}

		uint32_t Hash(const MultisampleState& state, uint32_t hash)
		{
			hash = utl::murmur_hash(utl::enum_cast(state.sample_count), hash);
			hash = utl::murmur_hash(state.alpha_to_coverage, hash);
			hash = utl::murmur_hash(state.alpha_to_one, hash);
			return hash;
		}

		uint32_t Hash(const DepthStencilState& state, uint32_t hash)
		{
			hash = utl::murmur_hash(state.depth_test, hash);
			hash = utl::murmur_hash(state.depth_write, hash);
			hash = utl::murmur_hash(utl::enum_cast(state.depth_compare_op), hash);

			hash = utl::murmur_hash(state.stencil_test, hash);
			hash = utl::murmur_hash(utl::enum_cast(state.stencil_front_compare_op), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.front_fail), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.depth_front_fail), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.front_pass), hash);

			hash = utl::murmur_hash(utl::enum_cast(state.stencil_back_compare_op), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.back_fail), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.depth_back_fail), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.back_pass), hash);

			return hash;
		}

		uint32_t Hash(const ClearColor& color, uint32_t hash)
		{
			for (int32_t i = 0; i < color.color.size(); ++i)
			{
				hash = utl::murmur_hash(color.color[i], hash);
			}

			hash = utl::murmur_hash(color.depth, hash);
			hash = utl::murmur_hash(color.stencil, hash);

			return hash;
		}

		uint32_t Hash(const ColorBlendAttachment& attachment, uint32_t hash)
		{
			hash = utl::murmur_hash(attachment.enable, hash);
			hash = utl::murmur_hash(utl::enum_cast(attachment.src_color), hash);
			hash = utl::murmur_hash(utl::enum_cast(attachment.dst_color), hash);
			hash = utl::murmur_hash(utl::enum_cast(attachment.color_blend_op), hash);

			hash = utl::murmur_hash(utl::enum_cast(attachment.src_alpha), hash);
			hash = utl::murmur_hash(utl::enum_cast(attachment.dst_alpha), hash);
			hash = utl::murmur_hash(utl::enum_cast(attachment.alpha_blend_op), hash);

			//hash = utl::murmur_hash(static_cast<uint32_t>(attachment.color_mask), hash);

			return hash;
		}

		uint32_t Hash(const ColorBlendingState& state, uint32_t hash)
		{
			for (int32_t i = 0; i < state.attachment_count; ++i)
			{
				hash = Hash(state.attachments[i], hash);
			}

			return hash;
		}

		uint32_t Hash(const PipelineState& state, uint32_t hash)
		{
			hash = Hash(state.vertex_layout, hash);
			hash = Hash(state.rasterization_state, hash);
			hash = Hash(state.multisample_state, hash);
			hash = Hash(state.depth_stencil_state, hash);
			hash = Hash(state.color_blending_state, hash);

			for (int32_t i = 0; i < state.color_format_count; ++i)
			{
				hash = utl::murmur_hash(utl::enum_cast(state.color_formats[i]), hash);
			}

			hash = utl::murmur_hash(utl::enum_cast(state.depth_format), hash);
			hash = utl::murmur_hash(utl::enum_cast(state.stencil_format), hash);

			hash = utl::murmur_hash(utl::enum_cast(state.primitive_topology), hash);

			return hash;
		}



		uint32_t Hash(const SamplerCreateInfo& create_info, uint32_t hash)
		{
			hash = utl::murmur_hash(utl::enum_cast(create_info.mag_filter), hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.min_filter), hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.mip_map_mode), hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.address_u), hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.address_v), hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.address_w), hash);
			hash = utl::murmur_hash(create_info.mip_lod_bias, hash);
			hash = utl::murmur_hash(create_info.max_anisotropy, hash);
			hash = utl::murmur_hash(utl::enum_cast(create_info.depth_compare), hash);
			hash = utl::murmur_hash(create_info.min_lod, hash);
			hash = utl::murmur_hash(create_info.max_lod, hash);

			return hash;
		}

		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info)
		{
			switch (create_info.render_api)
			{
			case RenderApi::eVulkan:
				return std::make_unique<vulkan::VulkanContext>(create_info);
				break;
			default:
				break;
			}

			return nullptr;
		}
	}
}
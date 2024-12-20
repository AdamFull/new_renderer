#include "gfx_context.h"

#if D3D12_SUPPORTED
#include <directx12/D3D12Context.h>
#endif

#if VULKAN_SUPPORTED
#include <vulkan/VkContext.h>
#endif

#if OPENGL_SUPPORTED
#include <opengl/OGLContext.h>
#endif

#include "../../util/Enum.hpp"
#include "../../util/Hash.hpp"

namespace edge
{
	namespace gfx
	{
		uint32_t Hash(const VertexAttributeDescription& desc, uint32_t hash)
		{
			hash = murmur_hash(desc.location, hash);
			hash = murmur_hash(desc.binding, hash);
			hash = murmur_hash(enum_cast(desc.format), hash);
			hash = murmur_hash(desc.offset, hash);

			return hash;
		}

		uint32_t Hash(const VertexBindingDescription& desc, uint32_t hash)
		{
			hash = murmur_hash(desc.binding, hash);
			hash = murmur_hash(desc.stride, hash);

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
			hash = murmur_hash(state.depth_clamp, hash);
			hash = murmur_hash(state.discard_enable, hash);
			hash = murmur_hash(enum_cast(state.fill_mode), hash);
			//hash = murmur_hash(static_cast<uint32_t>(state.cull_mode), hash);
			hash = murmur_hash(enum_cast(state.front_face), hash);

			return hash;
		}

		uint32_t Hash(const MultisampleState& state, uint32_t hash)
		{
			hash = murmur_hash(enum_cast(state.sample_count), hash);
			hash = murmur_hash(state.alpha_to_coverage, hash);
			hash = murmur_hash(state.alpha_to_one, hash);
			return hash;
		}

		uint32_t Hash(const DepthStencilState& state, uint32_t hash)
		{
			hash = murmur_hash(state.depth_test, hash);
			hash = murmur_hash(state.depth_write, hash);
			hash = murmur_hash(enum_cast(state.depth_compare_op), hash);

			hash = murmur_hash(state.stencil_test, hash);
			hash = murmur_hash(enum_cast(state.stencil_front_compare_op), hash);
			hash = murmur_hash(enum_cast(state.front_fail), hash);
			hash = murmur_hash(enum_cast(state.depth_front_fail), hash);
			hash = murmur_hash(enum_cast(state.front_pass), hash);

			hash = murmur_hash(enum_cast(state.stencil_back_compare_op), hash);
			hash = murmur_hash(enum_cast(state.back_fail), hash);
			hash = murmur_hash(enum_cast(state.depth_back_fail), hash);
			hash = murmur_hash(enum_cast(state.back_pass), hash);

			return hash;
		}

		uint32_t Hash(const ClearColor& color, uint32_t hash)
		{
			for (int32_t i = 0; i < color.color.size(); ++i)
			{
				hash = murmur_hash(color.color[i], hash);
			}

			hash = murmur_hash(color.depth, hash);
			hash = murmur_hash(color.stencil, hash);

			return hash;
		}

		uint32_t Hash(const ColorBlendAttachment& attachment, uint32_t hash)
		{
			hash = murmur_hash(attachment.enable, hash);
			hash = murmur_hash(enum_cast(attachment.src_color), hash);
			hash = murmur_hash(enum_cast(attachment.dst_color), hash);
			hash = murmur_hash(enum_cast(attachment.color_blend_op), hash);

			hash = murmur_hash(enum_cast(attachment.src_alpha), hash);
			hash = murmur_hash(enum_cast(attachment.dst_alpha), hash);
			hash = murmur_hash(enum_cast(attachment.alpha_blend_op), hash);

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

			//for (int32_t i = 0; i < state.color_format_count; ++i)
			//{
			//	hash = murmur_hash(enum_cast(state.color_formats[i]), hash);
			//}
			//
			//hash = murmur_hash(enum_cast(state.depth_format), hash);
			//hash = murmur_hash(enum_cast(state.stencil_format), hash);

			hash = murmur_hash(enum_cast(state.primitive_topology), hash);

			return hash;
		}



		uint32_t Hash(const SamplerCreateInfo& create_info, uint32_t hash)
		{
			hash = murmur_hash(enum_cast(create_info.mag_filter), hash);
			hash = murmur_hash(enum_cast(create_info.min_filter), hash);
			hash = murmur_hash(enum_cast(create_info.mip_map_mode), hash);
			hash = murmur_hash(enum_cast(create_info.address_u), hash);
			hash = murmur_hash(enum_cast(create_info.address_v), hash);
			hash = murmur_hash(enum_cast(create_info.address_w), hash);
			hash = murmur_hash(create_info.mip_lod_bias, hash);
			hash = murmur_hash(create_info.max_anisotropy, hash);
			hash = murmur_hash(enum_cast(create_info.depth_compare), hash);
			hash = murmur_hash(create_info.min_lod, hash);
			hash = murmur_hash(create_info.max_lod, hash);

			return hash;
		}

		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info)
		{
			switch (create_info.render_api)
			{
#if D3D12_SUPPORTED
			case RenderApi::eDirectx12:
				return std::make_unique<d3d12::D3D12Context>(create_info);
				break;
#endif
#if VULKAN_SUPPORTED
			case RenderApi::eVulkan:
				return std::make_unique<vulkan::VulkanContext>(create_info);
				break;
#endif
#if OPENGL_SUPPORTED
			case RenderApi::eOpenGL:
				return std::make_unique<opengl::OpenGLContext>(create_info);
				break;
#endif
			default:
				break;
			}

			return nullptr;
		}
	}
}
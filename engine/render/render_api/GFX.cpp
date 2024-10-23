#include <GFX.h>

#include <vulkan/VkContext.h>
#include <hash.hpp>

namespace engine
{
	namespace gfx
	{
		uint32_t Hash(const SamplerCreateInfo& create_info)
		{
			uint32_t hash{ 0u };

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

		std::unique_ptr<GFXContext> CreateGFXContext(const GFXContextCreateInfo& create_info)
		{
			switch (create_info.render_api)
			{
			case RenderApi::eVulkan:
				return std::make_unique<vulkan::VkGFXContext>(create_info);
				break;
			default:
				break;
			}

			return nullptr;
		}
	}
}
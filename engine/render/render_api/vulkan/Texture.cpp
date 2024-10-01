#include <engine/render/render_api/vulkan/Texture.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/ImageView.h> 
#include <engine/render/render_api/vulkan/VkRenderDevice.h> 

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			inline bool IsDepthOnlyFormat(vk::Format format)
			{
				return format == vk::Format::eD16Unorm ||
					format == vk::Format::eD32Sfloat;
			}

			inline bool IsDepthStencilFormat(vk::Format format)
			{
				return format == vk::Format::eD16UnormS8Uint ||
					format == vk::Format::eD24UnormS8Uint ||
					format == vk::Format::eD32SfloatS8Uint;
			}

			inline bool IsDepthFormat(vk::Format format)
			{
				return IsDepthOnlyFormat(format) || IsDepthStencilFormat(format);
			}

			inline bool IsStencilFormat(vk::Format format)
			{
				return !IsDepthOnlyFormat(format) || !IsDepthStencilFormat(format);
			}

			inline vk::ImageUsageFlags GetTextureUsageFlags(vk::Format format, ETextureCreateFlags flags)
			{
				vk::ImageUsageFlags usage_flags{};

				if (flags & ETextureCreateFlagBits::eRenderTarget)
				{
					usage_flags |= ((IsDepthStencilFormat(format) || IsDepthOnlyFormat(format)) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment);
				}

				if (flags & ETextureCreateFlagBits::eTransferable)
					usage_flags |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

				if (flags & ETextureCreateFlagBits::eSamplable)
					usage_flags |= vk::ImageUsageFlagBits::eSampled;

				if (flags & ETextureCreateFlagBits::eStorage)
					usage_flags |= vk::ImageUsageFlagBits::eStorage;

				return usage_flags;
			}

			inline vk::ImageAspectFlags GetTextureAspectFlags(vk::Format format)
			{
				vk::ImageAspectFlags aspect_flags{};

				if (IsDepthFormat(format))
					aspect_flags |= vk::ImageAspectFlagBits::eDepth;
				if (IsDepthStencilFormat(format))
					aspect_flags |= vk::ImageAspectFlagBits::eStencil;

				if (aspect_flags == vk::ImageAspectFlagBits::eNone)
					aspect_flags = vk::ImageAspectFlagBits::eColor;

				return aspect_flags;
			}

			u64 CalculateSamplerHash(const FVkSamplerCreateInfo& sampler_ci)
			{
				static std::hash<u32> hasher;
				u64 hash{ 0ull };

				hash ^= hasher(utl::enum_cast(sampler_ci.magFilter));
				hash ^= hasher(utl::enum_cast(sampler_ci.minFilter));
				hash ^= hasher(utl::enum_cast(sampler_ci.addressMode));
				hash ^= hasher(utl::cast<u32>(sampler_ci.enableAnisotropy));
				hash ^= hasher(utl::cast<u32>(sampler_ci.enableCompareOp));
				hash ^= hasher(sampler_ci.mipLevels);

				return hash;
			}

			Texture::Texture(VkRenderDevice& render_device, const FVkTextureCreateInfo& create_info)
			{
				properties.format = create_info.format;
				properties.flags = create_info.flags;
				properties.size = create_info.size;
				properties.mip_levels = create_info.mipLevels;
				properties.array_layers = create_info.arrayLayers;
				properties.sample_count = create_info.sampleCount;
				properties.total_mips = create_info.mipLevels;

				auto image_builder = ImageBuilder(vk::Extent3D{ create_info.size.width, create_info.size.height, create_info.size.depth });
				auto image_view_builder = ImageViewBuilder();

#ifndef BUILD_WITHOUT_DEBUGGIND
				image_builder.SetDebugName(create_info.name);
#endif

				if (create_info.size.depth > 1 || (create_info.flags & ETextureCreateFlagBits::e3D))
					image_builder.SetImageType(vk::ImageType::e3D);
				else
				{
					if (create_info.flags & ETextureCreateFlagBits::e1D)
						image_builder.SetImageType(vk::ImageType::e1D);
					else if (create_info.flags & ETextureCreateFlagBits::e2D)
						image_builder.SetImageType(vk::ImageType::e2D);
					else if (create_info.flags & ETextureCreateFlagBits::e3D)
						image_builder.SetImageType(vk::ImageType::e3D);
					else
						image_builder.SetImageType(vk::ImageType::e2D);
				}

				image_builder.SetMipLevels(create_info.mipLevels);

				properties.total_layers = create_info.arrayLayers;
				if ((create_info.flags & ETextureCreateFlagBits::eArray) && (create_info.flags & ETextureCreateFlagBits::eCubemap))
				{
					properties.total_layers = create_info.arrayLayers * 6u;
					image_view_builder.SetViewType(vk::ImageViewType::eCubeArray);
				}
				else if (create_info.flags & ETextureCreateFlagBits::eArray)
					image_view_builder.SetViewType(vk::ImageViewType::e2DArray);
				else if (create_info.flags & ETextureCreateFlagBits::eCubemap)
				{
					properties.total_layers = 6u;
					image_view_builder.SetViewType(vk::ImageViewType::eCube);
				}
				else if (create_info.flags & ETextureCreateFlagBits::e3D)
					image_view_builder.SetViewType(vk::ImageViewType::e3D);
				else
				{
					properties.total_layers = 1u;
					image_view_builder.SetViewType(vk::ImageViewType::e2D);
				}

				image_builder.SetArrayLayers(properties.total_layers);

				image_builder.SetVmaUsage(vma::MemoryUsage::eUnknown);
				image_builder.SetFormat(create_info.format);
				image_builder.SetUsage(GetTextureUsageFlags(create_info.format, create_info.flags));
				image_builder.SetSampleCount(create_info.sampleCount);
				image_builder.SetSharing(vk::SharingMode::eExclusive);
				image_builder.SetFlags((create_info.flags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage);

				image = image_builder.BuildUnique(render_device);

				image_view_builder.SetArrayLayerlCount(properties.total_layers);
				image_view_builder.SetFormat(create_info.format);

				auto image_aspect = GetTextureAspectFlags(create_info.format);
				if (image_aspect & vk::ImageAspectFlagBits::eStencil)
					image_aspect = vk::ImageAspectFlagBits::eDepth;

				FVkImageViewCreateInfo image_view_ci;
				image_view_builder.SetAspectMask(image_aspect);
				image_view_builder.SetBaseArrayLayer(0u);
				image_view_builder.SetArrayLayerlCount(properties.total_layers);

				// At first Create default view that contain all subresources
				image_view_builder.SetBaseMipLevel(0u);
				image_view_builder.SetMipLevelCount(create_info.mipLevels);

#ifndef BUILD_WITHOUT_DEBUGGIND
				image_view_builder.SetDebugName(std::format("{}_main", create_info.name));
#endif
				base = image_view_builder.BuildUnique(render_device, *image);

				mips.resize(create_info.mipLevels);
				for (u32 mip_index = 0u; mip_index < create_info.mipLevels; ++mip_index)
				{
					auto& current_mip = mips[mip_index];
					image_view_builder.SetBaseMipLevel(mip_index);
					image_view_builder.SetMipLevelCount(1u);

#ifndef BUILD_WITHOUT_DEBUGGIND
					image_view_builder.SetDebugName(std::format("{}[{}]", create_info.name, mip_index));
#endif
					current_mip = image_view_builder.BuildUnique(render_device, *image);
				}

//				if (texture_ci.CreateSampler)
//				{
//					FVkSamplerCreateInfo sampler_ci{};
//					sampler_ci.addressMode = texture_ci.addressMode;
//					sampler_ci.enableAnisotropy = texture_ci.enableAnisotropy;
//					sampler_ci.enableCompareOp = is_depth_format(image_ci.ci.format);
//					sampler_ci.minFilter = sampler_ci.magFilter = texture_ci.filter;
//					sampler_ci.mipLevels = texture_ci.mipLevels;
//#ifndef BUILD_WITHOUT_DEBUGGIND
//					sampler_ci.name = texture_ci.name;
//#endif
//					u64 key = calculate_sampler_hash(sampler_ci);
//					auto& sampler = sampler_cache[key];
//
//					if (!sampler)
//						render_device->CreateSampler(sampler_ci, sampler);
//
//					texture.sampler = key;
//				}
//
//				texture.isAlive = true;
			}

			Texture::~Texture()
			{

			}

			Image& Texture::GetImage()
			{
				return *image;
			}

			const Image& Texture::GetImage() const
			{
				return *image;
			}
		}
	}
}
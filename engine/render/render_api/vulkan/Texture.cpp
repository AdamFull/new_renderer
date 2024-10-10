#include <engine/render/render_api/vulkan/Texture.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/ImageView.h> 
#include <engine/render/render_api/vulkan/Sampler.h> 
#include <engine/render/render_api/vulkan/VkRenderDevice.h> 
#include <engine/render/render_api/vulkan/VkCommon.h> 

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

			TextureBuilder::TextureBuilder() :
				image_format{ vk::Format::eUndefined }, texture_flags{ 0u }, sizing_policy{ ERenderTargetSizePolicy::eScalable },
				image_extent{ 1u, 1u, 1u }, scale_factor{ 1.f }, mip_count{ 1u }, layer_count{ 1u },
				sample_count{ vk::SampleCountFlagBits::e1 }, sampler_id{ ~0u }, external_image{ nullptr },
				external_image_view{ nullptr }
			{
			}

			TextureBuilder& TextureBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetFormat(vk::Format fmt)
			{
				image_format = fmt;
				return *this;
			}

			TextureBuilder& TextureBuilder::AddFlag(ETextureCreateFlagBits flag)
			{
				texture_flags |= flag;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetFlags(ETextureCreateFlags flags)
			{
				texture_flags = flags;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetSizingPolicy(ERenderTargetSizePolicy policy)
			{
				sizing_policy = policy;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetExtent(vk::Extent3D extent)
			{
				image_extent = extent;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetScaleFactor(float scale)
			{
				scale_factor = scale;
				return *this;
			}


			TextureBuilder& TextureBuilder::SetMipCount(uint32_t count)
			{
				mip_count = count;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetLayerCount(uint32_t count)
			{
				layer_count = count;
				return *this;
			}


			TextureBuilder& TextureBuilder::SetSampleCount(vk::SampleCountFlagBits count)
			{
				sample_count = count;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetSampler(uint32_t id)
			{
				sampler_id = id;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetExternalImage(vk::Image handle)
			{
				external_image = handle;
				return *this;
			}

			TextureBuilder& TextureBuilder::SetExternalImageView(vk::ImageView handle)
			{
				external_image_view = handle;
				return *this;
			}


			Texture TextureBuilder::Build(RenderDevice& render_device) const
			{
				return Texture(render_device, *this);
			}

			std::unique_ptr<Texture> TextureBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<Texture>(render_device, *this);
			}


			Texture::Texture(RenderDevice& render_device, const TextureBuilder& builder) :
				debug_name{ builder.debug_name }, flags{ builder.texture_flags },
				sizing_policy{ builder.sizing_policy }, scale_factor{ builder.scale_factor },
				sampler{ builder.sampler_id }
			{
				auto image_builder = ImageBuilder(builder.image_extent);
				image_builder.SetExternalHandle(builder.external_image);

				auto image_view_builder = ImageViewBuilder();
				image_view_builder.SetExternalHandle(builder.external_image_view);

				image_builder.SetDebugName(debug_name);

				if (builder.image_extent.depth > 1 || (flags & ETextureCreateFlagBits::e3D))
					image_builder.SetImageType(vk::ImageType::e3D);
				else
				{
					if (flags & ETextureCreateFlagBits::e1D)
						image_builder.SetImageType(vk::ImageType::e1D);
					else if (flags & ETextureCreateFlagBits::e2D)
						image_builder.SetImageType(vk::ImageType::e2D);
					else if (flags & ETextureCreateFlagBits::e3D)
						image_builder.SetImageType(vk::ImageType::e3D);
					else
						image_builder.SetImageType(vk::ImageType::e2D);
				}

				image_builder.SetMipLevels(builder.mip_count);

				u32 total_layers = builder.layer_count;
				if ((flags & ETextureCreateFlagBits::eArray) && (flags & ETextureCreateFlagBits::eCubemap))
				{
					total_layers = builder.layer_count * 6u;
					image_view_builder.SetViewType(vk::ImageViewType::eCubeArray);
				}
				else if (flags & ETextureCreateFlagBits::eArray)
					image_view_builder.SetViewType(vk::ImageViewType::e2DArray);
				else if (flags & ETextureCreateFlagBits::eCubemap)
				{
					total_layers = 6u;
					image_view_builder.SetViewType(vk::ImageViewType::eCube);
				}
				else if (flags & ETextureCreateFlagBits::e3D)
					image_view_builder.SetViewType(vk::ImageViewType::e3D);
				else
				{
					total_layers = 1u;
					image_view_builder.SetViewType(vk::ImageViewType::e2D);
				}

				image_builder.SetArrayLayers(total_layers);

				image_builder.SetVmaUsage(vma::MemoryUsage::eUnknown);
				image_builder.SetFormat(builder.image_format);
				image_builder.SetUsage(GetTextureUsageFlags(builder.image_format, flags));
				image_builder.SetSampleCount(builder.sample_count);
				image_builder.SetSharing(vk::SharingMode::eExclusive);
				image_builder.SetFlags((flags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage);

				image = image_builder.BuildUnique(render_device);

				image_view_builder.SetArrayLayerlCount(total_layers);
				image_view_builder.SetFormat(builder.image_format);

				auto image_aspect = GetTextureAspectFlags(builder.image_format);
				if (image_aspect & vk::ImageAspectFlagBits::eStencil)
					image_aspect = vk::ImageAspectFlagBits::eDepth;

				image_view_builder.SetAspectMask(image_aspect);
				image_view_builder.SetBaseArrayLayer(0u);
				image_view_builder.SetArrayLayerlCount(total_layers);

				// At first Create default view that contain all subresources
				image_view_builder.SetBaseMipLevel(0u);
				image_view_builder.SetMipLevelCount(builder.mip_count);
				image_view_builder.SetDebugName(std::format("{}_main", builder.debug_name));

				base = image_view_builder.BuildUnique(render_device, *image);

				// Need to create additional views only if it's controlled from this class
				if (!builder.external_image && !builder.external_image_view)
				{
					mips.resize(builder.mip_count);
					for (u32 mip_index = 0u; mip_index < builder.mip_count; ++mip_index)
					{
						auto& current_mip = mips[mip_index];
						image_view_builder.SetBaseMipLevel(mip_index);
						image_view_builder.SetMipLevelCount(1u);

						image_view_builder.SetDebugName(std::format("{}[{}]", builder.debug_name, mip_index));
						current_mip = image_view_builder.BuildUnique(render_device, *image);
					}
				}
			}

			Texture::Texture(Texture& other, vk::Extent3D new_extent) :
				debug_name{ other.debug_name }, flags{ other.flags },
				sizing_policy{ other.sizing_policy }, scale_factor{ other.scale_factor },
				sampler{ other.sampler }
			{
				// TODO TEXTURE
				auto image_builder = ImageBuilder(new_extent);
				auto image_view_builder = ImageViewBuilder();

				debug_name = other.debug_name;
				image_builder.SetDebugName(other.debug_name);

				flags = other.flags;

				auto& other_image = other.image;
				auto& other_image_view = other.base;

				image_builder.SetImageType(other_image->GetType());

				auto base_subresource_range = other.base->GetSubresourceRange();
				image_builder.SetMipLevels(base_subresource_range.levelCount);
				image_builder.SetArrayLayers(other_image->GetArrayLayerCount());
				image_view_builder.SetViewType(other_image_view->GetType());

				image_builder.SetVmaUsage(vma::MemoryUsage::eUnknown);
				image_builder.SetFormat(other_image->GetFormat());
				image_builder.SetUsage(GetTextureUsageFlags(other_image->GetFormat(), flags));
				image_builder.SetSampleCount(other_image->GetSampleCount());
				image_builder.SetSharing(vk::SharingMode::eExclusive);
				image_builder.SetFlags((flags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage);

				image = image_builder.BuildUnique(other_image->GetDevice());

				image_view_builder.SetArrayLayerlCount(other_image->GetArrayLayerCount());
				image_view_builder.SetFormat(other_image->GetFormat());

				auto image_aspect = GetTextureAspectFlags(other_image->GetFormat());
				if (image_aspect & vk::ImageAspectFlagBits::eStencil)
					image_aspect = vk::ImageAspectFlagBits::eDepth;

				image_view_builder.SetAspectMask(image_aspect);
				image_view_builder.SetBaseArrayLayer(0u);
				image_view_builder.SetArrayLayerlCount(other_image->GetArrayLayerCount());

				// At first Create default view that contain all subresources
				image_view_builder.SetBaseMipLevel(0u);
				image_view_builder.SetMipLevelCount(base_subresource_range.levelCount);
				image_view_builder.SetDebugName(std::format("{}_main", debug_name));

				base = image_view_builder.BuildUnique(other_image->GetDevice(), *image);

				mips.resize(base_subresource_range.levelCount);
				for (u32 mip_index = 0u; mip_index < base_subresource_range.levelCount; ++mip_index)
				{
					auto& current_mip = mips[mip_index];
					image_view_builder.SetBaseMipLevel(mip_index);
					image_view_builder.SetMipLevelCount(1u);

					image_view_builder.SetDebugName(std::format("{}[{}]", debug_name, mip_index));
					current_mip = image_view_builder.BuildUnique(other_image->GetDevice(), *image);
				}
			}

			Texture::~Texture()
			{
				base.reset();
				mips.clear();
				image.reset();
			}

			Image& Texture::GetImage()
			{
				return *image;
			}

			const Image& Texture::GetImage() const
			{
				return *image;
			}

			ImageView& Texture::GetImageView(uint32_t idx)
			{
				if (idx == ~0u)
					return *base;

				return *mips[idx];
			}

			const ImageView& Texture::GetImageView(uint32_t idx) const
			{
				if (idx == ~0u)
					return *base;

				return *mips[idx];
			}

			const std::vector<std::unique_ptr<ImageView>>& Texture::GetImageViews()
			{
				return mips;
			}

			ETextureCreateFlags const& Texture::GetFlags() const
			{
				return flags;
			}

			uint32_t Texture::GetSamplerID() const
			{
				return sampler;
			}

			ERenderTargetSizePolicy Texture::GetSizingPolicy() const
			{
				return sizing_policy;
			}

			float Texture::GetScaleFactor() const
			{
				return scale_factor;
			}

			vk::ImageLayout Texture::GetFinalLayout() const
			{
				auto usage_flags = image->GetUsage();

				if (usage_flags & vk::ImageUsageFlagBits::eStorage)
					return vk::ImageLayout::eGeneral; // vk::ImageLayout::eShaderReadOnlyOptimal
				else if (usage_flags & vk::ImageUsageFlagBits::eColorAttachment)
				{
					if (usage_flags & vk::ImageUsageFlagBits::eTransferSrc)
						return vk::ImageLayout::eTransferSrcOptimal;
					else if (usage_flags & vk::ImageUsageFlagBits::eSampled)
						return vk::ImageLayout::eShaderReadOnlyOptimal;
					else if (usage_flags & vk::ImageUsageFlagBits::eInputAttachment)
					{
						if (usage_flags & vk::ImageUsageFlagBits::eSampled)
							return vk::ImageLayout::eShaderReadOnlyOptimal;
						else
							return vk::ImageLayout::eColorAttachmentOptimal;
					}
				}
				else if (usage_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment)
				{
					if (usage_flags & vk::ImageUsageFlagBits::eSampled)
						return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
					else
						return vk::ImageLayout::eDepthStencilAttachmentOptimal;
				}
				else
				{
					if (usage_flags & vk::ImageUsageFlagBits::eSampled && usage_flags & vk::ImageUsageFlagBits::eTransferDst)
						return vk::ImageLayout::eShaderReadOnlyOptimal;
				}

				return vk::ImageLayout::ePresentSrcKHR;
			}

			vk::ImageLayout Texture::GetIntermediateLayout() const
			{
				auto usage_flags = image->GetUsage();

				if (usage_flags & vk::ImageUsageFlagBits::eStorage)
					return vk::ImageLayout::eGeneral;
				else if (usage_flags & vk::ImageUsageFlagBits::eColorAttachment)
					return vk::ImageLayout::eColorAttachmentOptimal;
				else if (usage_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment)
				{
					// TODO: check is that stencil only target
					auto format = image->GetFormat();
					bool is_depth_stencil_attachment = IsDepthStencilFormat(format);
					return is_depth_stencil_attachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : IsDepthOnlyFormat(format) ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eStencilAttachmentOptimal;
				}

				return vk::ImageLayout::eUndefined;
			}

			vk::AttachmentLoadOp Texture::GetLoadOp()
			{
				return vk::AttachmentLoadOp::eClear;
			}

			vk::AttachmentStoreOp Texture::GetStoreOp()
			{
				bool transferable = utl::cast<bool>(flags & ETextureCreateFlagBits::eTransferable);
				bool samplable = utl::cast<bool>(flags & ETextureCreateFlagBits::eSamplable);

				if (transferable || samplable)
					return vk::AttachmentStoreOp::eStore;

				return vk::AttachmentStoreOp::eDontCare;
			}

			void Texture::MakeImageLayoutTranslation(FVkImageTranslationInfo& translation_info, const FResourceBarrierInfo& barrier)
			{
				auto& texture_subresource = GetImageView(barrier.subresourceIndex);

				translation_info.subresourceRange = texture_subresource.GetSubresourceRange();

				vk::ImageLayout old_image_layout{ vk::ImageLayout::eUndefined }, new_image_layout{ vk::ImageLayout::eUndefined };

				if (barrier.barrierType == ERenderResourceBarrierType::eIntermediate)
					new_image_layout = GetIntermediateLayout();
				else if (barrier.barrierType == ERenderResourceBarrierType::eFinal)
					new_image_layout = GetFinalLayout();

				old_image_layout = texture_subresource.GetImageLayout();
				texture_subresource.SetImageLayout(new_image_layout);
				translation_info.handle = Handle(*image);
				translation_info.subresourceRange = texture_subresource.GetSubresourceRange();

				// TODO: Fix here required. Sometimes i need to process all layers and levels
				if (barrier.subresourceIndex == ~0u)
				{
					for (auto& mip : mips)
						mip->SetImageLayout(new_image_layout);
				}

				ERenderResourceAccessFlagBits src_resource_access{};
				ERenderResourceAccessFlagBits dst_resource_access{};
				unpackDependencyBarrier(barrier.dependencyType, src_resource_access, dst_resource_access);

				translation_info.srcStageMask = get_translation_stage_flags(old_image_layout, barrier.srcPassType);
				translation_info.srcAccessMask = get_translation_access_flags(old_image_layout, barrier.srcPassType, src_resource_access);
				translation_info.oldLayout = old_image_layout;
				translation_info.dstStageMask = get_translation_stage_flags(new_image_layout, barrier.dstPassType);
				translation_info.dstAccessMask = get_translation_access_flags(new_image_layout, barrier.dstPassType, dst_resource_access);
				translation_info.newLayout = new_image_layout;
			}

			void Texture::MakeImageLayoutTranslation(FVkMemoryTranslationInfo& translation_info, const FResourceBarrierInfo& barrier)
			{
				auto& texture_subresource = GetImageView(barrier.subresourceIndex);

				ERenderResourceAccessFlagBits src_resource_access{};
				ERenderResourceAccessFlagBits dst_resource_access{};
				unpackDependencyBarrier(barrier.dependencyType, src_resource_access, dst_resource_access);

				translation_info.srcStageMask = get_translation_stage_flags(texture_subresource.GetImageLayout(), barrier.srcPassType);
				translation_info.srcAccessMask = get_translation_access_flags(texture_subresource.GetImageLayout(), barrier.srcPassType, src_resource_access);
				translation_info.dstStageMask = get_translation_stage_flags(texture_subresource.GetImageLayout(), barrier.dstPassType);
				translation_info.dstAccessMask = get_translation_access_flags(texture_subresource.GetImageLayout(), barrier.dstPassType, dst_resource_access);
			}

			FVkImageTranslationInfo Texture::GetTranslationInfo(const FVkTextureTranslationInfo& translation_info)
			{
				FVkImageTranslationInfo image_layout_translation{};
				image_layout_translation.srcStageMask = get_translation_stage_flags(translation_info.oldLayout);
				image_layout_translation.srcAccessMask = get_translation_access_flags(translation_info.oldLayout);
				image_layout_translation.oldLayout = translation_info.oldLayout;
				image_layout_translation.dstStageMask = get_translation_stage_flags(translation_info.newLayout);
				image_layout_translation.dstAccessMask = get_translation_access_flags(translation_info.newLayout);
				image_layout_translation.newLayout = translation_info.newLayout;
				image_layout_translation.handle = Handle(*image);

				image_layout_translation.subresourceRange = base->GetSubresourceRange();

				return image_layout_translation;
			}

			vk::DescriptorImageInfo Texture::GetDescriptor(uint32_t level)
			{
				return {};
			}
		}
	}
}
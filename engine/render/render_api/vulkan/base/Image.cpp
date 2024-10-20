#include <base/Image.h>
#include <base/Device.h>
#include <base/DebugUtils.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			ImageBuilder::ImageBuilder(vk::Extent3D const& extent) :
				// Better reasonable defaults than vk::ImageCreateInfo default ctor
				AllocationBuilder{ vk::ImageCreateInfo{ {}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm, extent, 1u, 1u } }
			{
			}

			ImageBuilder::ImageBuilder(vk::Extent2D const& extent) :
				ImageBuilder(vk::Extent3D{ extent.width, extent.height, 1u })
			{
			}

			ImageBuilder::ImageBuilder(uint32_t width, uint32_t height, uint32_t depth) :
				ImageBuilder(vk::Extent3D{ width, height, depth })
			{
			}

			ImageBuilder& ImageBuilder::SetFormat(vk::Format format)
			{
				create_info.format = format;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetImageType(vk::ImageType type)
			{
				create_info.imageType = type;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetArrayLayers(uint32_t layers)
			{
				create_info.arrayLayers = layers;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetMipLevels(uint32_t levels)
			{
				create_info.mipLevels = levels;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetSampleCount(vk::SampleCountFlagBits sample_count)
			{
				create_info.samples = sample_count;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetTiling(vk::ImageTiling tiling)
			{
				create_info.tiling = tiling;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetUsage(vk::ImageUsageFlags usage)
			{
				create_info.usage = usage;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetFlags(vk::ImageCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			ImageBuilder& ImageBuilder::SetImplicitSharingMode()
			{
				if (create_info.queueFamilyIndexCount != 0)
				{
					create_info.sharingMode = vk::SharingMode::eConcurrent;
				}
				return *this;
			}

			ImageBuilder& ImageBuilder::SetExternalHandle(vk::Image handle)
			{
				external_handle = handle;
				return *this;
			}

			Image ImageBuilder::Build(RenderDevice& render_device) const
			{
				return Image(render_device, *this);
			}

			std::unique_ptr<Image> ImageBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<Image>(render_device, *this);
			}


			Image::Image(RenderDevice& render_device, ImageBuilder const& builder) :
				VulkanResource{ nullptr },
				Allocation{ render_device, builder.alloc_create_info },
				create_info{ builder.create_info }
			{
				if (!builder.external_handle)
				{
					handle = render_device.CreateImage(*this, builder.create_info, builder.debug_name);
				}
				else
				{
					external_handle = true;
					handle = builder.external_handle;
				}
				
				subresource.arrayLayer = create_info.arrayLayers;
				subresource.mipLevel = create_info.mipLevels;
			}

			Image::Image(Image&& other) noexcept :
				VulkanResource{ std::move(other) },
				Allocation{ std::move(other) },
				create_info(std::exchange(other.create_info, {})),
				subresource(std::exchange(other.subresource, {}))
			{
			}

			Image::~Image()
			{
				if (!external_handle)
				{
					render_device.Destroy(*this, handle);
				}
			}

			uint8_t* Image::Map()
			{
				if (create_info.tiling != vk::ImageTiling::eLinear)
				{
					log_warning("Mapping image memory that is not linear");
				}
				return Allocation::Map();
			}

			vk::ImageType Image::GetType() const
			{
				return create_info.imageType;
			}

			const vk::Extent3D& Image::GetExtent() const
			{
				return create_info.extent;
			}

			vk::Format Image::GetFormat() const
			{
				return create_info.format;
			}

			vk::SampleCountFlagBits Image::GetSampleCount() const
			{
				return create_info.samples;
			}

			vk::ImageUsageFlags Image::GetUsage() const
			{
				return create_info.usage;
			}

			vk::ImageTiling Image::GetTiling() const
			{
				return create_info.tiling;
			}

			vk::ImageSubresource Image::GetSubresource() const
			{
				return subresource;
			}

			uint32_t Image::GetMipLevelCount() const
			{
				return create_info.mipLevels;
			}

			uint32_t Image::GetArrayLayerCount() const
			{
				return create_info.arrayLayers;
			}
		}
	}
}
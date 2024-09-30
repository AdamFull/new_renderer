#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

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

			Image ImageBuilder::Build(VkRenderDevice& context) const
			{
				return Image(context, *this);
			}

			std::unique_ptr<Image> ImageBuilder::BuildUnique(VkRenderDevice& context) const
			{
				return std::make_unique<Image>(context, *this);
			}


			Image::Image(VkRenderDevice& context, ImageBuilder const& builder) :
				VulkanResource{ nullptr },
				Allocation{ context.GetAllocator(), builder.alloc_create_info },
				create_info{ builder.create_info },
				device{ context.GetLogicalDevice() }
			{
				handle = allocator.CreateImage(*this, create_info);
				subresource.arrayLayer = create_info.arrayLayers;
				subresource.mipLevel = create_info.mipLevels;

				if (!builder.debug_name.empty())
				{
					context.GetDebugUtils().SetDebugName(handle, builder.debug_name.c_str());
				}
			}

			Image::Image(Image&& other) noexcept :
				VulkanResource{ std::move(other) },
				Allocation{ std::move(other) },
				device{ other.device },
				create_info(std::exchange(other.create_info, {})),
				subresource(std::exchange(other.subresource, {}))
			{
			}

			Image::~Image()
			{
				allocator.DestroyImage(*this, handle);
			}

			uint8_t* Image::Map()
			{
				if (create_info.tiling != vk::ImageTiling::eLinear)
				{
					log_warning("Mapping image memory that is not linear");
				}
				return Allocation::Map();
			}

			Device& Image::GetDevice()
			{
				return device;
			}

			Device const& Image::GetDevice() const
			{
				return device;
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

			uint32_t Image::GetArrayLayerCount() const
			{
				return create_info.arrayLayers;
			}
		}
	}
}
#include <engine/render/render_api/vulkan/Buffer.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/VkCommon.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{

			inline vk::DeviceSize CalculateAlignedSize(vk::DeviceSize allocation_size, vk::DeviceSize minimal_alignment)
			{
				if (minimal_alignment > 0ull)
					return (allocation_size + minimal_alignment - 1ull) & ~(minimal_alignment - 1ull);
				return allocation_size;
			}

			BufferBuilder::BufferBuilder(vk::DeviceSize allocation_size, vk::DeviceSize suballocation_count, vk::DeviceSize minimal_alignment) :
				AllocationBuilder{ vk::BufferCreateInfo{ {}, 0ull } }, suballocation_count{ suballocation_count }
			{
				create_info.size = CalculateAlignedSize(allocation_size, minimal_alignment) * suballocation_count;
			}

			Buffer BufferBuilder::Build(RenderDevice& render_device) const
			{
				Finalize(render_device);
				return Buffer(render_device, *this);
			}

			std::unique_ptr<Buffer> BufferBuilder::BuildUnique(RenderDevice& render_device) const
			{
				Finalize(render_device);
				return std::make_unique<Buffer>(render_device, *this);
			}

			BufferBuilder& BufferBuilder::SetFlags(vk::BufferCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			BufferBuilder& BufferBuilder::SetImplicitSharingMode()
			{
				if (create_info.queueFamilyIndexCount != 0)
				{
					create_info.sharingMode = vk::SharingMode::eConcurrent;
				}

				return *this;
			}

			BufferBuilder& BufferBuilder::SetSharingMode(vk::SharingMode sharing_mode)
			{
				create_info.sharingMode = sharing_mode;
				return *this;
			}

			BufferBuilder& BufferBuilder::SetUsage(vk::BufferUsageFlags usage)
			{
				create_info.usage = usage;
				return *this;
			}

			void BufferBuilder::Finalize(RenderDevice& render_device) const
			{
				if (create_info.sharingMode == vk::SharingMode::eConcurrent)
				{
					auto& device = render_device.GetLogicalDevice();
					auto family_count = device.GetQueueCount();

					std::iota(queue_families.begin(), queue_families.begin() + family_count, 0u);
					create_info.queueFamilyIndexCount = family_count;
					create_info.pQueueFamilyIndices = &queue_families[0];
				}
			}



			Buffer::Buffer(RenderDevice& render_device, BufferBuilder const& builder) :
				VulkanResource{ nullptr },
				Allocation{ render_device, builder.alloc_create_info },
				size{ builder.create_info.size },
				suballocation_count{ builder.suballocation_count }
			{
				handle = render_device.CreateBuffer(*this, builder.create_info, builder.debug_name);
			}

			Buffer::~Buffer()
			{
				render_device.Destroy(*this, handle);
			}

			void Buffer::CopyTo(vk::CommandBuffer& current, Buffer& dst_buffer, const std::vector<vk::BufferCopy2KHR>& regions)
			{
				vk::CopyBufferInfo2KHR copy_buffer{};
				copy_buffer.srcBuffer = handle;
				copy_buffer.dstBuffer = dst_buffer.handle;
				copy_buffer.regionCount = static_cast<uint32_t>(regions.size());
				copy_buffer.pRegions = &regions[0];

				current.copyBuffer2KHR(copy_buffer);
			}

			void Buffer::CopyTo(vk::CommandBuffer& current, vk::Image& image, vk::ImageLayout layout, const utl::vector<vk::BufferImageCopy2KHR>& regions)
			{
				vk::CopyBufferToImageInfo2KHR copy_info{};
				copy_info.srcBuffer = handle;
				copy_info.dstImage = image;
				copy_info.dstImageLayout = layout;
				copy_info.regionCount = static_cast<uint32_t>(regions.size());
				copy_info.pRegions = regions.data();

				current.copyBufferToImage2KHR(&copy_info);
			}

			uint64_t Buffer::GetDeviceAddress() const
			{
				return render_device.GetBufferDeviceAddress(handle);
			}

			vk::DeviceSize Buffer::GetSize() const
			{
				return size;
			}

			vk::DeviceSize Buffer::GetSuballocationSize() const
			{
				return size / suballocation_count;
			}

			void Buffer::MakeTranslation(vk::BufferMemoryBarrier2KHR& buffer_barrier, const FResourceBarrierInfo& barrier)
			{
				ERenderResourceAccessFlagBits src_resource_access{};
				ERenderResourceAccessFlagBits dst_resource_access{};
				unpackDependencyBarrier(barrier.dependencyType, src_resource_access, dst_resource_access);

				buffer_barrier.buffer = handle;
				buffer_barrier.size = size;
				buffer_barrier.srcStageMask = get_translation_stage_flags(barrier.srcPassType);
				buffer_barrier.srcAccessMask = get_translation_access_flags(barrier.srcPassType, src_resource_access);
				buffer_barrier.dstStageMask = get_translation_stage_flags(barrier.dstPassType);
				buffer_barrier.dstAccessMask = get_translation_access_flags(barrier.dstPassType, dst_resource_access);
			}
		}
	}
}
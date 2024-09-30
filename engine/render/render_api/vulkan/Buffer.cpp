#include <engine/render/render_api/vulkan/Buffer.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

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

			Buffer BufferBuilder::Build(VkRenderDevice& render_device) const
			{
				return Buffer(render_device, *this);
			}

			std::unique_ptr<Buffer> BufferBuilder::BuildUnique(VkRenderDevice& render_device) const
			{
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



			Buffer::Buffer(VkRenderDevice& render_device, BufferBuilder const& builder) :
				VulkanResource{ nullptr },
				Allocation{ render_device.GetAllocator(), builder.alloc_create_info },
				size{ builder.create_info.size },
				suballocation_count{ builder.suballocation_count }
			{
				handle = allocator.CreateBuffer(*this, builder.create_info);
				if (!builder.debug_name.empty())
				{
					render_device.GetDebugUtils().SetDebugName(handle, builder.debug_name.c_str());
				}
			}

			Buffer::~Buffer()
			{
				if (allocator)
				{
					allocator.DestroyBuffer(*this, handle);
				}
			}

			uint64_t Buffer::GetDeviceAddress() const
			{
				auto& device = allocator.GetDevice();
				return Handle(device).getBufferAddressKHR(handle);
			}

			vk::DeviceSize Buffer::GetSize() const
			{
				return size;
			}

			vk::DeviceSize Buffer::GetSuballocationSize() const
			{
				return size / suballocation_count;
			}
		}
	}
}
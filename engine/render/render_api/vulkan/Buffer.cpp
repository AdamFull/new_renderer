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
			BufferBuilder::BufferBuilder(vk::DeviceSize size) :
				AllocationBuilder{ vk::BufferCreateInfo{ {}, size } }
			{
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

			BufferBuilder& BufferBuilder::SetUsage(vk::BufferUsageFlagBits usage)
			{
				create_info.usage = usage;
				return *this;
			}



			Buffer::Buffer(VkRenderDevice& render_device, BufferBuilder const& builder) :
				VulkanResource{ nullptr },
				Allocation{ render_device.GetAllocator(), builder.alloc_create_info },
				size{ builder.create_info.size }
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
		}
	}
}
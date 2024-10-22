#include <VkBuffer.h>
#include <VkDevice.h>
#include <VkDebug.h>
#include <VkCommon.h>

#include <util/Util.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanBuffer::VulkanBuffer(Allocator& allocator, const vk::BufferCreateInfo& buffer_create_info, const vma::AllocationCreateInfo& allocation_create_info) :
			VulkanResource{ nullptr, nullptr },
			Allocation(allocator, allocation_create_info),
			size{ buffer_create_info.size }
		{
			handle = allocator.CreateBuffer(*this, buffer_create_info);
		}

		VulkanBuffer::~VulkanBuffer()
		{
			allocator.DestroyBuffer(*this, handle);
		}

		uint8_t* VulkanBuffer::Map()
		{
			if (!persistent && !IsMapped())
			{
				auto result = Handle(allocator).mapMemory(allocation, reinterpret_cast<void**>(&mapped_data));
				if (result != vk::Result::eSuccess)
				{
					//throw VulkanException(result, "Failed to map memory.");
					throw std::runtime_error("Failed to map memory.");
				}

				assert(mapped_data);
			}
			return mapped_data;
		}

		void VulkanBuffer::Unmap()
		{
			if (!persistent && IsMapped())
			{
				Handle(allocator).unmapMemory(allocation);
				mapped_data = nullptr;
			}
		}

		uint64_t VulkanBuffer::Update(const void* data, uint64_t size, uint64_t offset)
		{
			if (persistent)
			{
				engine::util::Copy(mapped_data, offset, data, size, 0ull, std::execution::par);
				Flush();
			}
			else
			{
				Map();
				engine::util::Copy(mapped_data, offset, data, size, 0ull, std::execution::par);
				Flush();
				Unmap();
			}
			return size;
		}

		uint64_t VulkanBuffer::GetSize() const
		{
			return size;
		}

		uint64_t VulkanBuffer::GetAddress() const
		{
			auto& device = allocator.GetDevice();
			return Handle(device).getBufferAddressKHR(handle);
		}
	}
}
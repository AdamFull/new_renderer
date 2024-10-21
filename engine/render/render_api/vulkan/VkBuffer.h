#pragma once

#include <VkAllocator.h>
#include <ResourceStructures.h>

#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class VulkanBuffer final : public Buffer, public Allocation, public VulkanResource<vk::Buffer>
		{
		public:
			VulkanBuffer(const VulkanBuffer&) = delete;
			VulkanBuffer(VulkanBuffer&& other) = default;
			VulkanBuffer& operator=(const VulkanBuffer&) = delete;
			VulkanBuffer& operator=(VulkanBuffer&&) = default;

			VulkanBuffer(Allocator& allocator, const vk::BufferCreateInfo& buffer_create_info, const vma::AllocationCreateInfo& allocation_create_info);
			~VulkanBuffer();

			uint64_t Update(const void* data, uint64_t size, uint64_t offset = 0ull) override;

			uint64_t GetSize() const override;
			uint64_t GetAddress() const override;
		private:
			vk::DeviceSize size{ 0ull };
			vk::DeviceSize suballocation_count{ 0ull };
		};
	}
}
#pragma once

#include "VkAllocator.h"

#include <Base.h>

namespace engine
{
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

				VulkanBuffer(Allocator& allocator, const BufferCreateInfo& create_info);
				~VulkanBuffer();

				void SetResourceName(const char* name) override;

				/**
				 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
				 * @return Pointer to host visible memory
				 */
				uint8_t* Map() override;

				/**
				 * @brief Unmaps vulkan memory from the host visible address
				 */
				void Unmap() override;

				uint64_t Update(const void* data, uint64_t size, uint64_t offset = 0ull) override;

				uint64_t GetSize() const override;
				uint64_t GetAddress() const override;

				BufferUsageFlags GetUsageFlags() const override;
			private:
				BufferUsageFlags usage_flags{};
				vk::DeviceSize size{ 0ull };
				vk::DeviceSize suballocation_count{ 0ull };
			};
		}
	}
}
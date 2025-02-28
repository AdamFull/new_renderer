#pragma once

#include "VkAllocator.h"

#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class VulkanBuffer final : public IBuffer, public Allocation, public VulkanResource<vk::Buffer>
			{
			public:
				VulkanBuffer(const VulkanBuffer&) = delete;
				VulkanBuffer(VulkanBuffer&& other) = default;
				VulkanBuffer& operator=(const VulkanBuffer&) = delete;
				VulkanBuffer& operator=(VulkanBuffer&&) = default;

				VulkanBuffer(Allocator& allocator, const BufferCreateInfo& create_info);
				~VulkanBuffer();

				void SetResourceName(std::string_view name) const noexcept override;

				/**
				 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
				 * @return Pointer to host visible memory
				 */
				[[nodiscard]] auto Map() -> uint8_t* override;

				/**
				 * @brief Unmaps vulkan memory from the host visible address
				 */
				void Unmap() noexcept override;

				[[nodiscard]] auto Update(const void* data, uint64_t size, uint64_t offset = 0ull) -> uint64_t override;

				[[nodiscard]] auto GetSize() const noexcept -> uint64_t override;
				[[nodiscard]] auto GetAddress() const ->uint64_t override;

				[[nodiscard]] auto GetUsageFlags() const noexcept ->BufferUsageFlags override;
			private:
				BufferUsageFlags usage_flags{};
				vk::DeviceSize size{ 0ull };
				vk::DeviceSize suballocation_count{ 0ull };
			};
		}
	}
}
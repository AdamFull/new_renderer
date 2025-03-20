#pragma once

#include "VkAllocator.h"

#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class VulkanBuffer final : public IBuffer, public Allocation, public VulkanResource<vk::Buffer> {
			public:
				VulkanBuffer(const VulkanBuffer&) = delete;
				VulkanBuffer(VulkanBuffer&& other) = default;
				VulkanBuffer& operator=(const VulkanBuffer&) = delete;
				VulkanBuffer& operator=(VulkanBuffer&&) = default;

				VulkanBuffer(Allocator& allocator, const BufferCreateInfo& create_info);
				~VulkanBuffer();

				void set_resource_name(std::string_view name) const noexcept override;

				/**
				 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
				 * @return Pointer to host visible memory
				 */
				[[nodiscard]] auto map() -> uint8_t* override;

				/**
				 * @brief Unmaps vulkan memory from the host visible address
				 */
				void unmap() noexcept override;

				[[nodiscard]] auto update(const void* data, uint64_t size, uint64_t offset = 0ull) -> uint64_t override;

				[[nodiscard]] auto get_size() const noexcept -> uint64_t override {
					return size;
				}

				[[nodiscard]] auto get_address() const ->uint64_t override;

				[[nodiscard]] auto get_usage_flags() const noexcept ->BufferUsageFlags override;
			private:
				BufferUsageFlags usage_flags{};
				vk::DeviceSize size{ 0ull };
				vk::DeviceSize suballocation_count{ 0ull };
			};
		}
	}
}
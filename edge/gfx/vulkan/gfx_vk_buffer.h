#pragma once

#include "gfx_vk_allocator.h"

#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Buffer final : public IBuffer, public Allocation, public Resource<vk::Buffer> {
	public:
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) = default;
		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		Buffer(Allocator& allocator, const BufferCreateInfo& create_info);
		~Buffer();

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

		[[nodiscard]] auto get_address() const->uint64_t override;

		[[nodiscard]] auto get_usage_flags() const noexcept ->BufferUsageFlags override;
	private:
		BufferUsageFlags usage_flags{};
		vk::DeviceSize size{ 0ull };
		vk::DeviceSize suballocation_count{ 0ull };
	};
}
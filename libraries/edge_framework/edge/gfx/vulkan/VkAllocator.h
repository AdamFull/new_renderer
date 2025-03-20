#pragma once

#include <array>
#include <vector>
#include <span>

#include "VkResource.h"

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace edge::gfx::vulkan
{
	class Device;
	class Allocation;

	class Allocator final : public VulkanResource<vma::Allocator> {
	public:
		Allocator(Device& device);
		~Allocator();

		[[nodiscard]] vk::Buffer create_buffer(Allocation& allocation, vk::BufferCreateInfo const& create_info);
		void destroy_buffer(Allocation& allocation, vk::Buffer& handle);
		[[nodiscard]] vk::Image create_image(Allocation& allocation, vk::ImageCreateInfo const& create_info);
		void destroy_image(Allocation& allocation, vk::Image& image);

		[[nodiscard]] auto get_device() noexcept -> Device& {
			return device;
		}

	protected:
		void post_create(Allocation& allocation, vma::AllocationInfo const& allocation_info);
	private:
		Device& device;
	};

	class Allocation {
	public:
		friend class Allocator;

		Allocation(Allocator& allocator);

		Allocation(Allocation const&) = delete;
		Allocation(Allocation&& other) noexcept;

		Allocation& operator=(const Allocation&) = delete;
		Allocation& operator=(Allocation&&) = default;

		[[nodiscard]] auto get_data() const -> uint8_t const* {
			return mapped_data;
		}

		[[nodiscard]] auto get_memory() const->vk::DeviceMemory;

		auto get_device() const noexcept -> Device const& {
			return allocator.get_device();
		}

		auto get_device() noexcept -> Device& {
			return allocator.get_device();
		}

		/**
		 * @brief Flushes memory if it is HOST_VISIBLE and not HOST_COHERENT
		 */
		void flush(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);

		/**
		 * @brief Returns true if the memory is mapped, false otherwise
		 * @return mapping status
		 */
		auto is_mapped() const noexcept -> bool {
			return mapped_data != nullptr;
		}

		/**
		 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
		 * @return Pointer to host visible memory
		 */
		[[modiscard]] auto map_memory() -> uint8_t*;

		/**
		 * @brief Unmaps vulkan memory from the host visible address
		 */
		void unmap_memory();

		explicit operator bool() const noexcept {
			return allocation;
		}

		bool operator!() const noexcept {
			return !allocation;
		}

		auto get_usage() const noexcept -> vma::MemoryUsage {
			return alloc_create_info.usage;
		}

		auto get_allocator() noexcept -> Allocator& {
			return allocator;
		}
	protected:
		void clear();
	protected:
		Allocator& allocator;

		vma::AllocationCreateInfo alloc_create_info{};
		vma::Allocation allocation{ nullptr };
		uint8_t* mapped_data{ nullptr };
		bool coherent{ false };
		bool persistent{ false };
	};
}
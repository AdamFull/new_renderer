#pragma once

#include <array>
#include <vector>
#include <span>

#include "VkResource.h"

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class Allocation;

			class Allocator final : public VulkanResource<vma::Allocator>
			{
			public:
				Allocator(Device& device);
				~Allocator();

				[[nodiscard]] vk::Buffer CreateBuffer(Allocation& allocation, vk::BufferCreateInfo const& create_info);
				void DestroyBuffer(Allocation& allocation, vk::Buffer& handle);
				[[nodiscard]] vk::Image CreateImage(Allocation& allocation, vk::ImageCreateInfo const& create_info);
				void DestroyImage(Allocation& allocation, vk::Image& image);

				[[nodiscard]] auto GetDevice() noexcept -> Device&
				{
					return device;
				}

			protected:
				void PostCreate(Allocation& allocation, vma::AllocationInfo const& allocation_info);
			private:
				Device& device;
			};

			class Allocation
			{
			public:
				friend class Allocator;

				Allocation(Allocator& allocator);

				Allocation(Allocation const&) = delete;
				Allocation(Allocation&& other) noexcept;

				Allocation& operator=(const Allocation&) = delete;
				Allocation& operator=(Allocation&&) = default;

				[[nodiscard]] auto GetData() const -> uint8_t const*;
				[[nodiscard]] auto GetMemory() const ->vk::DeviceMemory;

				auto GetDevice() const noexcept -> Device const&
				{
					return allocator.GetDevice();
				}

				auto GetDevice() noexcept -> Device&
				{
					return allocator.GetDevice();
				}

				/**
				 * @brief Flushes memory if it is HOST_VISIBLE and not HOST_COHERENT
				 */
				void Flush(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);

				/**
				 * @brief Returns true if the memory is mapped, false otherwise
				 * @return mapping status
				 */
				auto IsMapped() const noexcept -> bool
				{
					return mapped_data != nullptr;
				}

				/**
				 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
				 * @return Pointer to host visible memory
				 */
				[[modiscard]] auto MapMemory() -> uint8_t*;

				/**
				 * @brief Unmaps vulkan memory from the host visible address
				 */
				void UnmapMemory();

				explicit operator bool() const noexcept
				{
					return allocation;
				}

				bool operator!() const noexcept
				{
					return !allocation;
				}

				auto GetUsage() const noexcept -> vma::MemoryUsage
				{
					return alloc_create_info.usage;
				}

				auto GetAllocator() noexcept -> Allocator&
				{
					return allocator;
				}
			protected:
				void Clear();
			protected:
				Allocator& allocator;

				vma::AllocationCreateInfo alloc_create_info{};
				vma::Allocation allocation{ nullptr };
				uint8_t* mapped_data{ nullptr };
				bool coherent{ false };
				bool persistent{ false };
			};
		}
	}
}
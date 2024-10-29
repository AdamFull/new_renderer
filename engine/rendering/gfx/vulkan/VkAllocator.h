#pragma once

#include <array>
#include <vector>
#include <span>

#include "VkResource.h"

#include <vma/vk_mem_alloc.hpp>

namespace engine
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

				Device& GetDevice();

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

				const uint8_t* GetData() const;
				vk::DeviceMemory GetMemory() const;

				Device const& GetDevice() const;
				Device& GetDevice();

				/**
				 * @brief Flushes memory if it is HOST_VISIBLE and not HOST_COHERENT
				 */
				void Flush(vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);

				/**
				 * @brief Returns true if the memory is mapped, false otherwise
				 * @return mapping status
				 */
				bool IsMapped() const;

				/**
				 * @brief Maps vulkan memory if it isn't already mapped to an host visible address
				 * @return Pointer to host visible memory
				 */
				uint8_t* MapMemory();

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

				vma::MemoryUsage GetUsage();
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
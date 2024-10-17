#pragma once

#include <array>
#include <vector>
#include <span>

#include <engine/render/render_api/vulkan/VkResource.h>

#include <vma/vk_mem_alloc.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;
			class Allocation;
			class RenderDevice;

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

			template<typename BuilderType, typename CreateInfoType>
			struct AllocationBuilder
			{
				vma::AllocationCreateInfo alloc_create_info{};
				std::string debug_name;
				mutable CreateInfoType create_info;

			protected:
				//AllocationBuilder(const AllocationBuilder& other) = delete;
				AllocationBuilder(const CreateInfoType& create_info) :
					create_info(create_info)
				{
					alloc_create_info.usage = vma::MemoryUsage::eAuto;
				};

			public:
				BuilderType& SetDebugName(const std::string& name)
				{
					debug_name = name;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetVmaUsage(vma::MemoryUsage usage)
				{
					alloc_create_info.usage = usage;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetVmaFlags(vma::AllocationCreateFlags flags)
				{
					alloc_create_info.flags = flags;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetVmaRequiredFlags(vk::MemoryPropertyFlags flags)
				{
					alloc_create_info.requiredFlags = flags;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetVmaPreferredFlags(vk::MemoryPropertyFlags flags)
				{
					alloc_create_info.preferredFlags = flags;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetMemoryTypeBits(uint32_t type_bits)
				{
					alloc_create_info.memoryTypeBits = type_bits;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetVmaPool(vma::Pool pool)
				{
					alloc_create_info.pool = pool;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetQueueFamilies(uint32_t count, const uint32_t* family_indices)
				{
					create_info.queueFamilyIndexCount = count;
					create_info.pQueueFamilyIndices = family_indices;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetSharing(vk::SharingMode sharing)
				{
					create_info.sharingMode = sharing;
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetImplicitSharingMode()
				{
					if (create_info.queueFamilyIndexCount != 0)
					{
						create_info.sharingMode = vk::SharingMode::eConcurrent;
					}
					else
					{
						create_info.sharingMode = vk::SharingMode::eExclusive;
					}
					return *static_cast<BuilderType*>(this);
				}

				BuilderType& SetQueueFamilies(const std::vector<uint32_t>& queue_families)
				{
					return SetQueueFamilies(static_cast<uint32_t>(queue_families.size()), queue_families.data());
				}
			};

			class Allocation
			{
			public:
				friend class Allocator;

				Allocation(RenderDevice& allocator, const vma::AllocationCreateInfo& create_info);

				Allocation(Allocation const&) = delete;
				Allocation(Allocation&& other) noexcept;

				Allocation& operator=(const Allocation&) = delete;
				Allocation& operator=(Allocation&&) = default;

				const uint8_t* GetData() const;
				vk::DeviceMemory GetMemory() const;

				RenderDevice& GetDevice();
				RenderDevice const& GetDevice() const;

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
				uint8_t* Map();

				/**
				 * @brief Unmaps vulkan memory from the host visible address
				 */
				void Unmap();

				/**
				 * @brief Copies byte data into the buffer
				 * @param data The data to copy from
				 * @param size The amount of bytes to copy
				 * @param offset The offset to start the copying into the mapped data
				 */
				size_t Update(const uint8_t* data, size_t size, size_t offset = 0ull);
				/**
				 * @brief Converts any non byte data into bytes and then updates the buffer
				 * @param data The data to copy from
				 * @param size The amount of bytes to copy
				 * @param offset The offset to start the copying into the mapped data
				 */
				size_t Update(void const* data, size_t size, size_t offset = 0ull);

				/**
				 * @todo Use the vk::ArrayBuffer class to collapse some of these templates
				 * @brief Copies a vector of items into the buffer
				 * @param data The data vector to upload
				 * @param offset The offset to start the copying into the mapped data
				 */
				template <typename _Ty>
				size_t Update(std::vector<_Ty> const& data, size_t offset = 0ull)
				{
					return Update(data.data(), data.size() * sizeof(_Ty), offset);
				}

				template <typename _Ty, size_t N>
				size_t Update(std::array<_Ty, N> const& data, size_t offset = 0ull)
				{
					return Update(data.data(), data.size() * sizeof(_Ty), offset);
				}

				template<typename _Ty>
				size_t Update(std::span<_Ty> data, size_t offset = 0ull)
				{
					return Update(data.data(), data.size() * sizeof(_Ty), offset);
				}

				/**
				 * @brief Copies an object as byte data into the buffer
				 * @param object The object to convert into byte data
				 * @param offset The offset to start the copying into the mapped data
				 */
				template <typename _Ty>
				size_t ConvertAndUpdate(const _Ty& object, size_t offset = 0ull)
				{
					return Update(reinterpret_cast<const uint8_t*>(&object), sizeof(_Ty), offset);
				}

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
				RenderDevice& render_device;

				vma::AllocationCreateInfo alloc_create_info{};
				vma::Allocation allocation{ nullptr };
				uint8_t* mapped_data{ nullptr };
				bool coherent{ false };
				bool persistent{ false };
			};
		}
	}
}
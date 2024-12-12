#include "VkAllocator.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"

#include <logger.hpp>
#include <util/util.h>

#include <volk.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			Allocator::Allocator(Device& device) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vma::VulkanFunctions vma_vulkan_func{};
				vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
				vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

				auto& physical = device.GetGpu();

				vma::AllocatorCreateInfo allocator_info{};
				allocator_info.pVulkanFunctions = &vma_vulkan_func;
				allocator_info.physicalDevice = Handle(physical);
				allocator_info.device = Handle(device);
				allocator_info.instance = Handle(physical.GetInstance());
				allocator_info.pAllocationCallbacks = allocation_callbacks;

				bool can_get_memory_requirements = device.IsExtensionSupported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
				bool has_dedicated_allocation = device.IsExtensionSupported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
				if (can_get_memory_requirements && has_dedicated_allocation && device.IsEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eKhrDedicatedAllocation;
				}

				if (device.IsExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && device.IsEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eBufferDeviceAddress;
				}

				if (device.IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) && device.IsEnabled(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eExtMemoryBudget;
				}

				if (device.IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) && device.IsEnabled(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eExtMemoryPriority;
				}

				if (device.IsExtensionSupported(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) && device.IsEnabled(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eKhrBindMemory2;
				}

				if (device.IsExtensionSupported(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) && device.IsEnabled(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME))
				{
					allocator_info.flags |= vma::AllocatorCreateFlagBits::eAmdDeviceCoherentMemory;
				}

				vk::Result result = vma::createAllocator(&allocator_info, &handle);
				if (result != vk::Result::eSuccess)
				{
					//throw VulkanException{ result, "Cannot create allocator" };
					throw std::runtime_error{ "Cannot create allocator" };
				}
			}

			Allocator::~Allocator()
			{
				if (handle)
				{
					vma::TotalStatistics stats;
					handle.calculateStatistics(&stats);
					log_info("Total device memory leaked: {} bytes.", stats.total.statistics.allocationBytes);
					handle.destroy();
					handle = nullptr;
				}
			}

			[[nodiscard]] vk::Buffer Allocator::CreateBuffer(Allocation& allocation, vk::BufferCreateInfo const& create_info)
			{
				vk::Buffer buffer{ nullptr };
				vma::AllocationInfo allocation_info{};
				auto result = handle.createBuffer(&create_info, &allocation.alloc_create_info, &buffer, &allocation.allocation, &allocation_info);

				if (result != vk::Result::eSuccess)
				{
					//throw VulkanException{ result, "Cannot create Buffer" };
					throw std::runtime_error{ "Cannot create Buffer" };
				}

				PostCreate(allocation, allocation_info);
				return buffer;
			}

			void Allocator::DestroyBuffer(Allocation& allocation, vk::Buffer& buffer)
			{
				if (buffer && allocation)
				{
					allocation.UnmapMemory();
					handle.destroyBuffer(buffer, allocation.allocation);
					allocation.Clear();
				}
			}

			[[nodiscard]] vk::Image Allocator::CreateImage(Allocation& allocation, vk::ImageCreateInfo const& create_info)
			{
				assert(0 < create_info.mipLevels && "Images should have at least one level");
				assert(0 < create_info.arrayLayers && "Images should have at least one layer");
				assert(0 < (uint32_t)create_info.usage && "Images should have at least one usage type");

				vk::Image image{ nullptr };
				vma::AllocationInfo allocation_info{};

#if 0
				// If the image is an attachment, prefer dedicated memory
				constexpr VkImageUsageFlags attachment_only_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
				if (create_info.usage & attachment_only_flags)
				{
					alloc_create_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
				}

				if (create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
				{
					alloc_create_info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
				}
#endif
				auto result = handle.createImage(&create_info, &allocation.alloc_create_info, &image, &allocation.allocation, &allocation_info);

				if (result != vk::Result::eSuccess)
				{
					//throw VulkanException{ result, "Cannot create Image" };
					throw std::runtime_error{ "Cannot create Image" };
				}

				PostCreate(allocation, allocation_info);
				return image;
			}



			void Allocator::DestroyImage(Allocation& allocation, vk::Image& image)
			{
				if (image && allocation)
				{
					allocation.UnmapMemory();
					handle.destroyImage(image, allocation.allocation);
					allocation.Clear();
				}
			}

			Device& Allocator::GetDevice()
			{
				return device;
			}

			void Allocator::PostCreate(Allocation& allocation, vma::AllocationInfo const& allocation_info)
			{
				vk::MemoryPropertyFlags memory_properties;
				handle.getAllocationMemoryProperties(allocation.allocation, &memory_properties);
				allocation.coherent = (memory_properties & vk::MemoryPropertyFlagBits::eHostCoherent) == vk::MemoryPropertyFlagBits::eHostCoherent;
				allocation.mapped_data = static_cast<uint8_t*>(allocation_info.pMappedData);
				allocation.persistent = allocation.mapped_data;
			}


			Allocation::Allocation(Allocator& allocator) :
				allocator{ allocator }
			{
			}

			Allocation::Allocation(Allocation&& other) noexcept :
				allocator{ other.allocator }, alloc_create_info{ other.alloc_create_info }
			{
			}

			const uint8_t* Allocation::GetData() const
			{
				return mapped_data;
			}

			vk::DeviceMemory Allocation::GetMemory() const
			{
				vma::AllocationInfo alloc_info;
				Handle(allocator).getAllocationInfo(allocation, &alloc_info);
				return alloc_info.deviceMemory;
			}

			Device const& Allocation::GetDevice() const
			{
				return allocator.GetDevice();
			}

			Device& Allocation::GetDevice()
			{
				return allocator.GetDevice();
			}

			void Allocation::Flush(vk::DeviceSize offset, vk::DeviceSize size)
			{
				if (!coherent)
				{
					Handle(allocator).flushAllocation(allocation, offset, size);
				}
			}

			bool Allocation::IsMapped() const
			{
				return mapped_data != nullptr;
			}

			uint8_t* Allocation::MapMemory()
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

			void Allocation::UnmapMemory()
			{
				if (!persistent && IsMapped())
				{
					Handle(allocator).unmapMemory(allocation);
					mapped_data = nullptr;
				}
			}

			void Allocation::Clear()
			{
				mapped_data = nullptr;
				persistent = false;
				alloc_create_info = vma::AllocationCreateInfo{};
			}

			vma::MemoryUsage Allocation::GetUsage()
			{
				return alloc_create_info.usage;
			}

			Allocator& Allocation::GetAllocator()
			{
				return allocator;
			}
		}
	}
}
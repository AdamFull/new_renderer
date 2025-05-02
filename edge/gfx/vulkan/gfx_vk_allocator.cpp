#include "gfx_vk_allocator.h"
#include "gfx_vk_physical_device.h"
#include "gfx_vk_device.h"

#include "../../util/Logger.h"
#include "../../util/utl_base.h"

#include <volk.h>

namespace edge::gfx::vulkan
{
	Allocator::Allocator(Device& device) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		VmaVulkanFunctions vma_vulkan_func{};
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		auto& physical = device.get_gpu();

		VmaAllocatorCreateInfo allocator_info{};
		allocator_info.pVulkanFunctions = &vma_vulkan_func;
		allocator_info.physicalDevice = physical.get_handle();
		allocator_info.device = device.get_handle();
		allocator_info.instance = physical.get_instance().get_handle();
		allocator_info.pAllocationCallbacks = (VkAllocationCallbacks*)allocation_callbacks;

		// NOTE: Nsight graphics using VkImportMemoryHostPointerEXT that cannot be used with dedicated memory allocation
		bool can_get_memory_requirements = device.is_extension_supported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		//bool has_dedicated_allocation = device.is_extension_supported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
		bool has_dedicated_allocation = false;
		if (can_get_memory_requirements && has_dedicated_allocation && device.is_enabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		}

		if (device.is_extension_supported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && device.is_enabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}

		if (device.is_extension_supported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) && device.is_enabled(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		}

		if (device.is_extension_supported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) && device.is_enabled(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
		}

		if (device.is_extension_supported(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) && device.is_enabled(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
		}

		if (device.is_extension_supported(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) && device.is_enabled(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)) {
			allocator_info.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
		}

		VkResult result = vmaCreateAllocator(&allocator_info, &handle_);
		if (result != VK_SUCCESS) {
			//throw VulkanException{ result, "Cannot create allocator" };
			EDGE_LOGE("[VkAllocator] Cannot create allocator");
		}
	}

	Allocator::~Allocator() {
		if (handle_) {
			VmaTotalStatistics stats;
			vmaCalculateStatistics(handle_, &stats);
			EDGE_LOGI("[VkAllocator] Total device memory leaked: {} bytes.", stats.total.statistics.allocationBytes);
			vmaDestroyAllocator(handle_);
			handle_ = VK_NULL_HANDLE;
		}
	}

	[[nodiscard]] vk::Buffer Allocator::create_buffer(Allocation& allocation, vk::BufferCreateInfo const& create_info) {
		VkBuffer buffer{ VK_NULL_HANDLE };
		VmaAllocationInfo allocation_info{};
		
		VkBufferCreateInfo buffer_create_info = static_cast<VkBufferCreateInfo>(create_info);
		auto result = vmaCreateBuffer(handle_, &buffer_create_info, &allocation.alloc_create_info, 
			&buffer, &allocation.allocation, &allocation_info);

		if (result != VK_SUCCESS) {
			EDGE_LOGE("[VkAllocator] Cannot create Buffer");
		}

		post_create(allocation, allocation_info);
		return vk::Buffer(buffer);
	}

	void Allocator::destroy_buffer(Allocation& allocation, vk::Buffer& buffer) {
		if (buffer && allocation) {
			allocation.unmap_memory();
			vmaDestroyBuffer(handle_, (VkBuffer)buffer, allocation.allocation);
			allocation.clear();
		}
	}

	[[nodiscard]] vk::Image Allocator::create_image(Allocation& allocation, vk::ImageCreateInfo const& create_info) {
		assert(0 < create_info.mipLevels && "Images should have at least one level");
		assert(0 < create_info.arrayLayers && "Images should have at least one layer");
		assert(0 < (uint32_t)create_info.usage && "Images should have at least one usage type");

		VkImage image{ VK_NULL_HANDLE };
		VmaAllocationInfo allocation_info{};

#if 0
		// If the image is an attachment, prefer dedicated memory
		constexpr VkImageUsageFlags attachment_only_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		if (create_info.usage & attachment_only_flags) {
			alloc_create_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		if (create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
			alloc_create_info.preferredFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		}
#endif
		VkImageCreateInfo image_create_info = static_cast<VkImageCreateInfo>(create_info);
		auto result = vmaCreateImage(handle_, &image_create_info, &allocation.alloc_create_info, 
			&image, &allocation.allocation, &allocation_info);
		if (result != VK_SUCCESS) {
			EDGE_LOGE("[VkAllocator] Cannot create Image");
		}

		post_create(allocation, allocation_info);
		return vk::Image(image);
	}



	void Allocator::destroy_image(Allocation& allocation, vk::Image& image) {
		if (image && allocation) {
			allocation.unmap_memory();
			vmaDestroyImage(handle_, (VkImage)image, allocation.allocation);
			allocation.clear();
		}
	}

	void Allocator::post_create(Allocation& allocation, VmaAllocationInfo const& allocation_info) {
		VkMemoryPropertyFlags memory_properties;
		vmaGetAllocationMemoryProperties(handle_, allocation.allocation, &memory_properties);
		allocation.coherent = (memory_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		allocation.mapped_data = static_cast<uint8_t*>(allocation_info.pMappedData);
		allocation.persistent = allocation.mapped_data;
	}


	Allocation::Allocation(Allocator& allocator) :
		allocator{ allocator } {
	}

	Allocation::Allocation(Allocation&& other) noexcept :
		allocator{ other.allocator }, alloc_create_info{ other.alloc_create_info } {
	}

	vk::DeviceMemory Allocation::get_memory() const {
		VmaAllocationInfo alloc_info;
		vmaGetAllocationInfo(allocator.get_handle(), allocation, &alloc_info);
		return (vk::DeviceMemory)alloc_info.deviceMemory;
	}

	void Allocation::flush(vk::DeviceSize offset, vk::DeviceSize size) {
		if (!coherent) {
			vmaFlushAllocation(allocator.get_handle(), allocation, offset, size);
		}
	}

	uint8_t* Allocation::map_memory() {
		if (!persistent && !is_mapped()) {
			auto result = vmaMapMemory(allocator.get_handle(), allocation, reinterpret_cast<void**>(&mapped_data));
			if (result != VK_SUCCESS) {
				EDGE_LOGE("[VkAllocator] Failed to map memory.");
			}

			assert(mapped_data);
		}
		return mapped_data;
	}

	void Allocation::unmap_memory() {
		if (!persistent && is_mapped()) {
			vmaUnmapMemory(allocator.get_handle(), allocation);
			mapped_data = nullptr;
		}
	}

	void Allocation::clear() {
		mapped_data = nullptr;
		persistent = false;
		alloc_create_info = VmaAllocationCreateInfo{};
	}
}
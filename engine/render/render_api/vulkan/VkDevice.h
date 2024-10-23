#pragma once

#include <unordered_map>

#include <VkQueue.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class PhysicalDevice;

			class Device : public VulkanResource<vk::Device>
			{
			public:
				/**
				 * @brief Device constructor
				 * @param gpu A valid Vulkan physical device and the requested gpu features
				 * @param surface The surface
				 * @param debug_utils The debug utils to be associated to this device
				 * @param requested_extensions (Optional) List of required device extensions and whether support is optional or not
				 */
				Device(PhysicalDevice& gpu, vk::SurfaceKHR surface, std::vector<const char*> requested_extensions = {});

				Device(const Device&) = delete;
				Device(Device&&) = delete;

				~Device();

				Device& operator=(const Device&) = delete;
				Device& operator=(Device&&) = delete;

				PhysicalDevice const& GetGpu() const;

				VulkanQueue const& GetQueue(uint32_t queue_family_index, uint32_t queue_index) const;

				VulkanQueue const& GetQueueByFlags(vk::QueueFlags queue_flags, uint32_t queue_index) const;

				VulkanQueue const& GetQueueByPresent(uint32_t queue_index) const;

				size_t GetQueueCount() const;

				/**
				 * @brief Finds a suitable graphics queue to submit to
				 * @return The first present supported queue, otherwise just any graphics queue
				 */
				VulkanQueue const& GetSuitableGraphicsQueue() const;

				bool IsExtensionSupported(const char* extension) const;

				bool IsEnabled(std::string const& extension) const;

				uint32_t GetQueueFamilyIndex(vk::QueueFlagBits queue_flag) const;

			private:
				PhysicalDevice const& gpu;

				vk::AllocationCallbacks* allocation_callbacks{ nullptr };

				vk::SurfaceKHR surface{ nullptr };

				std::vector<const char*> enabled_extensions{};

				std::vector<std::vector<VulkanQueue>> queues;
			};
		}
	}
}
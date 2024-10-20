#pragma once

#include <unordered_map>

#include <base/Queue.h>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;
			class PhysicalDevice;

			class DeviceBuilder
			{
			public:
				std::vector<const char*> required_extensions;
				std::vector<const char*> optional_extensions;

				DeviceBuilder& SetRequiredExtensions(std::vector<const char*>&& extensions);
				DeviceBuilder& AddRequiredExtensions(const std::vector<const char*>& extensions);
				DeviceBuilder& AddRequiredExtension(const char* extension_name);

				DeviceBuilder& SetOptionalExtensions(std::vector<const char*>&& extensions);
				DeviceBuilder& AddOptionalExtensions(const std::vector<const char*>& extensions);
				DeviceBuilder& AddOptionalExtension(const char* extension_name);

				Device Build(PhysicalDevice& gpu, vk::SurfaceKHR& surface) const;
				std::unique_ptr<Device> BuildUnique(PhysicalDevice& gpu, vk::SurfaceKHR& surface) const;
			};

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

				Device(PhysicalDevice& gpu, vk::SurfaceKHR& surface, const DeviceBuilder& builder);

				Device(const Device&) = delete;
				Device(Device&&) = delete;

				~Device();

				Device& operator=(const Device&) = delete;
				Device& operator=(Device&&) = delete;

				PhysicalDevice const& GetGpu() const;

				Queue const& GetQueue(uint32_t queue_family_index, uint32_t queue_index) const;

				Queue const& GetQueueByFlags(vk::QueueFlags queue_flags, uint32_t queue_index) const;

				Queue const& GetQueueByPresent(uint32_t queue_index) const;

				size_t GetQueueCount() const;

				/**
				 * @brief Finds a suitable graphics queue to submit to
				 * @return The first present supported queue, otherwise just any graphics queue
				 */
				Queue const& GetSuitableGraphicsQueue() const;
				
				bool IsExtensionSupported(const char* extension) const;

				bool IsEnabled(std::string const& extension) const;

				uint32_t GetQueueFamilyIndex(vk::QueueFlagBits queue_flag) const;

			private:
				PhysicalDevice const& gpu;

				vk::AllocationCallbacks* allocation_callbacks{ nullptr };

				vk::SurfaceKHR surface{ nullptr };

				std::vector<const char*> enabled_extensions{};

				std::vector<std::vector<Queue>> queues;
			};
		}
	}
}
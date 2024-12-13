#include "VkDevice.h"
#include "VkPhysicalDevice.h"

#include <edge/util/Logger.h>

#include <volk.h>

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			Device::Device(PhysicalDevice& gpu, vk::SurfaceKHR surface, std::vector<const char*> requested_extensions) :
				VulkanResource{ nullptr, gpu.GetAllocationCallbacks() },
				gpu(gpu)
			{
				// Prepare the device queues
				auto& queue_family_properties = gpu.GetQueueFamilyProperties();
				std::vector<vk::DeviceQueueCreateInfo> queue_create_infos(queue_family_properties.size());
				std::vector<std::vector<float>> queue_priorities(queue_family_properties.size());

				for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties.size(); ++queue_family_index)
				{
					vk::QueueFamilyProperties const& queue_family_property = queue_family_properties[queue_family_index];

					if (gpu.HasHighPriorityGraphicsQueue())
					{
						uint32_t graphics_queue_family = GetQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
						if (graphics_queue_family == queue_family_index)
						{
							queue_priorities[queue_family_index].reserve(queue_family_property.queueCount);
							queue_priorities[queue_family_index].emplace_back(1.0f);
							for (uint32_t i = 1; i < queue_family_property.queueCount; i++)
							{
								queue_priorities[queue_family_index].emplace_back(0.5f);
							}
						}
						else
						{
							queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
						}
					}
					else
					{
						queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
					}

					vk::DeviceQueueCreateInfo& queue_create_info = queue_create_infos[queue_family_index];

					queue_create_info.queueFamilyIndex = queue_family_index;
					queue_create_info.queueCount = queue_family_property.queueCount;
					queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
				}

				// Check extensions to enable Vma Dedicated Allocation
				bool can_get_memory_requirements = IsExtensionSupported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
				bool has_dedicated_allocation = IsExtensionSupported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

				if (can_get_memory_requirements && has_dedicated_allocation)
				{
					enabled_extensions.emplace_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
					enabled_extensions.emplace_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

					EDGE_LOGI("Dedicated Allocation enabled");
				}

				// For performance queries, we also use host query reset since queryPool resets cannot
				// live in the same command buffer as beginQuery
				if (IsExtensionSupported(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) && IsExtensionSupported(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME))
				{
					auto perf_counter_features = gpu.RequestExtensionFeatures<vk::PhysicalDevicePerformanceQueryFeaturesKHR>();
					auto host_query_reset_features = gpu.RequestExtensionFeatures<vk::PhysicalDeviceHostQueryResetFeatures>();

					if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset)
					{
						enabled_extensions.emplace_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
						enabled_extensions.emplace_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
						EDGE_LOGI("Performance query enabled");
					}
				}

				enabled_extensions.insert(enabled_extensions.end(), requested_extensions.begin(), requested_extensions.end());

				if (enabled_extensions.size() > 0)
				{
					EDGE_LOGI("Device supports the following requested extensions:");
					for (auto& extension : enabled_extensions)
					{
						EDGE_LOGI("  \t{}", extension);
					}
				}

				vk::DeviceCreateInfo create_info({}, queue_create_infos, {}, enabled_extensions, &gpu.GetMutableRequestedFeatures());

				// Latest requested feature will have the pNext's all set up for device creation.
				create_info.pNext = gpu.GetExtensionFeatureChain();

				handle = Handle(gpu).createDevice(create_info);

				VULKAN_HPP_DEFAULT_DISPATCHER.init(handle);

				// Need to load volk for all the not-yet Vulkan-Hpp calls
				volkLoadDevice(handle);

				queues.resize(queue_family_properties.size());

				for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties.size(); ++queue_family_index)
				{
					vk::QueueFamilyProperties const& queue_family_property = queue_family_properties[queue_family_index];

					vk::Bool32 present_supported = Handle(gpu).getSurfaceSupportKHR(queue_family_index, surface);

					for (uint32_t queue_index = 0U; queue_index < queue_family_property.queueCount; ++queue_index)
					{
						queues[queue_family_index].emplace_back(*this, queue_family_index, queue_family_property, present_supported, queue_index);
					}
				}
			}

			Device::~Device()
			{
				if (handle)
				{
					handle.destroy();
				}
			}

			PhysicalDevice const& Device::GetGpu() const
			{
				return gpu;
			}

			VulkanQueue const& Device::GetQueue(uint32_t queue_family_index, uint32_t queue_index) const
			{
				return queues[queue_family_index][queue_index];
			}

			VulkanQueue const& Device::GetQueueByFlags(vk::QueueFlags required_queue_flags, uint32_t queue_index) const
			{
				for (size_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
				{
					VulkanQueue const& first_queue = queues[queue_family_index][0];

					vk::QueueFlags queue_flags = first_queue.GetProperties().queueFlags;
					uint32_t queue_count = first_queue.GetProperties().queueCount;

					if (((queue_flags & required_queue_flags) == required_queue_flags) && queue_index < queue_count)
					{
						return queues[queue_family_index][queue_index];
					}
				}

				throw std::runtime_error("Queue not found");
			}

			VulkanQueue const& Device::GetQueueByPresent(uint32_t queue_index) const
			{
				for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
				{
					VulkanQueue const& first_queue = queues[queue_family_index][0];

					uint32_t queue_count = first_queue.GetProperties().queueCount;

					if (first_queue.SupportPresent() && queue_index < queue_count)
					{
						return queues[queue_family_index][queue_index];
					}
				}

				throw std::runtime_error("Queue not found");
			}

			size_t Device::GetQueueCount() const
			{
				return queues.size();
			}

			VulkanQueue const& Device::GetSuitableGraphicsQueue() const
			{
				for (size_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
				{
					VulkanQueue const& first_queue = queues[queue_family_index][0];

					uint32_t queue_count = first_queue.GetProperties().queueCount;

					if (first_queue.SupportPresent() && 0 < queue_count)
					{
						return queues[queue_family_index][0];
					}
				}

				return GetQueueByFlags(vk::QueueFlagBits::eGraphics, 0);
			}

			bool Device::IsExtensionSupported(const char* extension) const
			{
				return gpu.IsExtensionSupported(extension);
			}

			bool Device::IsEnabled(std::string const& extension) const
			{
				return std::find_if(enabled_extensions.begin(), enabled_extensions.end(),
					[extension](const char* enabled_extension) { return extension == enabled_extension; }) != enabled_extensions.end();
			}

			uint32_t Device::GetQueueFamilyIndex(vk::QueueFlagBits queue_flag) const
			{
				const auto& queue_family_properties = gpu.GetQueueFamilyProperties();

				// Dedicated queue for compute
				// Try to find a queue family index that supports compute but not graphics
				if (queue_flag & vk::QueueFlagBits::eCompute)
				{
					for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
					{
						if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics))
						{
							return i;
							break;
						}
					}
				}

				// Dedicated queue for transfer
				// Try to find a queue family index that supports transfer but not graphics and compute
				if (queue_flag & vk::QueueFlagBits::eTransfer)
				{
					for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
					{
						if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
							!(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute))
						{
							return i;
							break;
						}
					}
				}

				// For other queue types or if no separate compute queue is present, return the first one to support the requested
				// flags
				for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
				{
					if (queue_family_properties[i].queueFlags & queue_flag)
					{
						return i;
						break;
					}
				}

				throw std::runtime_error("Could not find a matching queue family index");
			}

			void Device::SetDebugUtils(DebugUtils* debug_utils)
			{
				this->debug_utils = debug_utils;
			}

			DebugUtils const& Device::GetDebugUtils() const
			{
				return *debug_utils;
			}
		}
	}
}
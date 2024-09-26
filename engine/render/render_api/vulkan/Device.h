#pragma once

#include <unordered_map>

#include <engine/render/render_api/vulkan/Queue.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
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
				Device(PhysicalDevice& gpu, vk::AllocationCallbacks*, vk::SurfaceKHR surface, std::vector<const char*> requested_extensions = {});

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

				template<typename _CreateInfo, typename _ObjectHandle>
				vk::Result CreateObject(const _CreateInfo& create_info, _ObjectHandle& object)
				{ return {}; }

				template<>
				vk::Result CreateObject(const vk::PipelineCacheCreateInfo& create_info, vk::PipelineCache& object)
				{ return handle.createPipelineCache(&create_info, allocation_callbacks, &object); }

				template<>
				vk::Result CreateObject(const vk::SwapchainCreateInfoKHR& create_info, vk::SwapchainKHR& object)
				{ return handle.createSwapchainKHR(&create_info, allocation_callbacks, &object); }


				template<typename _ObjectType>
				void DestroyObject(_ObjectType& object)
				{
					throw std::runtime_error("Don't know how to destroy object of type");
				}

				template<>
				void DestroyObject(vk::PipelineCache& object)
				{
					handle.destroyPipelineCache(object, allocation_callbacks);
				}

				template<>
				void DestroyObject(vk::SwapchainKHR& object)
				{ handle.destroySwapchainKHR(object, allocation_callbacks); }

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
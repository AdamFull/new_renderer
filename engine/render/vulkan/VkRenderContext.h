#pragma once

#include <interface/RenderInterface.h>
#include <VkResourceStructures.h>

#include <format>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			// =================================================================================
			// =================================CVkRenderContext================================
			// =================================================================================
			class CVkRenderContext
			{
			public:
				friend class CVkRenderHandle;

				~CVkRenderContext();

				void create(const FRenderContextCreateInfo& context_ci);

				// VkCommandPool
				FRenderApiErrorInfo createCommandPool(const FVkCommandPoolCreateInfo& command_pool_ci, FCommandPool& command_pool);
				void destroyCommandPool(FCommandPool& command_pool);
				void destroyUnusedCommandPools();
				void destroyCommandPools();

				// VkCommandBuffer
				FRenderApiErrorInfo createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, FVkCommandBuffer& command_buffer);
				void destroyCommandBuffer(FVkCommandBuffer& command_buffer);
				void beginCommandBuffer(FVkCommandBuffer& command_buffer, vk::CommandBufferUsageFlags usage, uint32_t image_index);
				void endCommandBuffer(FVkCommandBuffer& command_buffer);

				// VkImage
				FRenderApiErrorInfo createImage(const FVkImageCreateInfo& image_ci, FVkImage& image);
				void destroyImage(FVkImage& image);

				// VkImageView
				FRenderApiErrorInfo createImageView(const FVkImageViewCreateInfo& image_view_ci, vk::ImageView& image_view);
				void destroyImageView(vk::ImageView& image_view);

				FRenderApiErrorInfo createSampler(const FVkSamplerCreateInfo& sampler_ci, vk::Sampler& sampler);
				void destroySampler(vk::Sampler& sampler);

				// VkBuffer
				FRenderApiErrorInfo createBuffer(const FVkBufferCreateInfo& buffer_ci, FVkBuffer& buffer);
				void destroyBuffer(FVkBuffer& buffer);
				FRenderApiErrorInfo mapBufferMemory(FVkBuffer& buffer, void* mappedMemory);
				void unmapBufferMemory(FVkBuffer& buffer);
				void flushBufferMemory(const FVkBuffer& buffer, vk::DeviceSize size, vk::DeviceSize offset);

				// VkDescriptorSetLayout
				FRenderApiErrorInfo createDescriptorSetLayout(const FVkDescriptorSetLayoutCreateInfo& layout_ci, vk::DescriptorSetLayout& descriptor_set_layout);
				void destroyDescriptorSetLayout(vk::DescriptorSetLayout& descriptor_set_layout);

				// VkShaderModule
				FRenderApiErrorInfo createShaderModule(const FVkShaderModuleCreateInfo& shader_module_ci, vk::ShaderModule& shader_module);
				void destroyShaderModule(vk::ShaderModule& shader_module);

				// VkPipelineLayout
				FRenderApiErrorInfo createPipelineLayout(const FVkPipelineLayoutCreateInfo& pipeline_layout_ci, vk::PipelineLayout& pipeline_layout);
				void destroyPipelineLayout(vk::PipelineLayout& pipeline_layout);

				// VkPipeline
				FRenderApiErrorInfo createPipeline(const FVkGraphicsPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline);
				FRenderApiErrorInfo createPipeline(const FVkComputePipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline);
				void destroyPipeline(vk::Pipeline& shader_object);

				FRenderApiErrorInfo createSemaphore(const FVkSemaphoreCreateInfo& semaphore_ci, vk::Semaphore& semaphore);
				FRenderApiErrorInfo createFence(const FVkFenceCreateInfo& fence_ci, vk::Fence& fence);


				vk::Result present(FVkCommandBuffer& command_buffer);
				vk::Result acquireNextImage(uint32_t& image_index);

				// Debug markers
				void debugBeginLabel(vk::CommandBuffer& command_buffer, const std::string& name, uint32_t color);
				void debugInsertLabel(vk::CommandBuffer& command_buffer, const std::string& name, uint32_t color);
				void endLabel(vk::CommandBuffer& command_buffer);

				void debugBeginLabel(vk::Queue& queue, const std::string& name, uint32_t color);
				void debugInsertLabel(vk::Queue& queue, const std::string& name, uint32_t color);
				void endLabel(vk::Queue& queue);

				template<class _Ty>
				void debugSetObjectName(const _Ty& object, const std::string& name)
				{
#ifndef BUILD_WITHOUT_DEBUGGIND
					if (!name.empty())
					{
						auto formatted_debug_object_name = std::format("{}:{}", vk::to_string(object.objectType), name);
						vk::DebugUtilsObjectNameInfoEXT object_name_info{};
						object_name_info.objectType = object.objectType;
						object_name_info.objectHandle = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(object));
						object_name_info.pObjectName = formatted_debug_object_name.c_str();

						logicalDevice.handle.setDebugUtilsObjectNameEXT(object_name_info);
					}
#endif
				}

				template<class _Ty>
				void debugSetObjectTag(const _Ty& object, uint64_t name, const void* data, size_t size)
				{
#ifndef BUILD_WITHOUT_DEBUGGIND
					vk::DebugUtilsObjectTagInfoEXT object_tag_info{};
					object_tag_info.objectType = object.objectType;
					object_tag_info.objectHandle = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(object));
					object_tag_info.tagName = name;
					object_tag_info.pTag = data;
					object_tag_info.tagSize = size;

					logicalDevice.handle.setDebugUtilsObjectTagEXT(object_tag_info);
#endif
				}

				// Getters
				const vk::DynamicLoader& getDynamicLoader() const;
				const vk::AllocationCallbacks* getAllocationCallbacks() const;

				const vk::Instance& getInstance() const;
				const vk::DebugUtilsMessengerEXT& getDebugMessanger() const;
				const vk::DebugReportCallbackEXT& getDebugReport() const;
				const vk::SurfaceKHR& getSurface() const;
				
				FVkPhysicalDevice& getPhysicalDevice();

				const vk::SampleCountFlagBits getMultisamplingCount() const;
				vk::Device& getLogicalDevice();

				vk::Queue& getQueue(uint32_t flag);

				vma::Allocator& getMemoryAllocator();

				const vk::PipelineCache& getPipelineCache() const;
				std::vector<char> getPipelineCacheData() const;

				const FSwapchain& getSwapchain() const;

				const FCommandPool& getCommandPool(uint32_t mask, const std::thread::id& thread_id = std::this_thread::get_id());
			protected:
				void registerAllocationCallbacks();
				FRenderApiErrorInfo createInstance(const FRenderContextCreateInfo& context_ci);
				void createDebugMessanger(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo createSurface(const FRenderContextCreateInfo& context_ci);
				void selectQueueFamilies(FVkPhysicalDevice& device_info);
				FRenderApiErrorInfo selectPhysicalDevice(const FRenderContextCreateInfo& context_ci);
				void prepareRequestedLogicalDeviceFeatures();
				FRenderApiErrorInfo createLogicalDevice(const FRenderContextCreateInfo& context_ci);
				void createMemoryAllocator(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo createPipelineCache(const FRenderContextCreateInfo& context_ci);
				FRenderApiErrorInfo createSwapchain(uint32_t surface_width, uint32_t surface_height);

				void cleanupSwapchain();

				template<class _Ty>
				_Ty& requestExtensionFeatures()
				{
					auto structure_type = _Ty::structureType;

					auto& physical_device = getPhysicalDevice();
					auto& extension_features = logicalDevice.extensionFeatures;

					auto found_feature = extension_features.find(structure_type);
					if (found_feature != extension_features.end())
						return *static_cast<_Ty*>(found_feature->second.get());

					vk::PhysicalDeviceFeatures2KHR physical_device_features2{};
					_Ty extension{};
					physical_device_features2.pNext = &extension;

					physical_device.handle.getFeatures2KHR(&physical_device_features2);

					extension_features.insert({ structure_type, std::make_shared<_Ty>(extension) });

					auto* extension_ptr = static_cast<_Ty*>(extension_features[structure_type].get());

					if (logicalDevice.lastExtensionFeature)
						extension_ptr->pNext = logicalDevice.lastExtensionFeature;
					logicalDevice.lastExtensionFeature = extension_ptr;

					return *extension_ptr;
				}
			private:
				vk::DynamicLoader dynamicLoader;
				vk::AllocationCallbacks* allocationCallbacks{ nullptr };

				vk::Instance vkInstance{ nullptr };
				vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
				vk::DebugReportCallbackEXT debugReport{ nullptr };
				vk::SurfaceKHR vkSurface{ nullptr };
				std::vector<std::string> avaliableLayers{};

				int32_t selectedPhysicalDevice{ -1 };
				std::vector<FVkPhysicalDevice> physicalDevices{};

				vk::SampleCountFlagBits requiredSampleCount{ vk::SampleCountFlagBits::e1 };
				FVkLogicalDevice logicalDevice{};

				vma::Allocator memoryAllocator{ nullptr };

				vk::PipelineCache pipelineCache{ nullptr };
				FSwapchain swapchain{};

				std::unordered_map<uint32_t, std::unordered_map<std::thread::id, FCommandPool>> commandPools{};
			};
		}
	}
}
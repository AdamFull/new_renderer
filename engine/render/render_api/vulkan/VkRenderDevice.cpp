#include "VkRenderDevice.h"

#include <sstream>
#include <set>

#include <udetail.hpp>
#include <VkCommon.h>

#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

using namespace engine::render;
using namespace engine::render::vulkan;

void* vk_malloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	void* ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void vk_free(void* pUserData, void* pMemory)
{
	free(pMemory);
}

void* vk_realloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return realloc(pOriginal, size);
}

void vk_malloc_notification(void* pUserData, size_t  size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}

void vk_free_notification(void* pUserData, size_t size, VkInternalAllocationType  allocationType, VkSystemAllocationScope allocationScope)
{
}

uint32_t get_api_version(ERenderApi version)
{
	switch (version)
	{
	case ERenderApi::eVulkan10:
		return VK_API_VERSION_1_0;
	case ERenderApi::eVulkan11:
		return VK_API_VERSION_1_1;
	case ERenderApi::eVulkan12:
		return VK_API_VERSION_1_2;
	case ERenderApi::eVulkan13:
		return VK_API_VERSION_1_3;
	}

	return 0u;
}


VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: log_verbose(pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: log_info(pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: log_warning(pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:  log_error(pCallbackData->pMessage); break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: break;
	}

	return VK_FALSE;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	vk::DebugReportObjectTypeEXT object_type = static_cast<vk::DebugReportObjectTypeEXT>(objectType);
	auto report_text = std::format("Report[{}:{}:{}]: {}", pLayerPrefix, vk::to_string(object_type), location, pMessage);

	switch (flags)
	{
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT: log_info(report_text); break;
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT: log_warning(report_text); break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT: log_warning(report_text); break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT: log_error(report_text); break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT: log_debug(report_text); break;
	}
	
	return VK_FALSE;
}

CVkRenderDevice::~CVkRenderDevice()
{
	destroyCommandPools();

	cleanupSwapchain();
	logicalDevice.handle.destroySwapchainKHR(swapchain.handle, allocationCallbacks);
	logicalDevice.handle.destroyPipelineCache(pipelineCache, allocationCallbacks);
	memoryAllocator.destroy();
	logicalDevice.handle.destroy(allocationCallbacks);
	vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger, allocationCallbacks);
	vkInstance.destroyDebugReportCallbackEXT(debugReport, allocationCallbacks);
	vkInstance.destroySurfaceKHR(vkSurface, allocationCallbacks);
	vkInstance.destroy(allocationCallbacks);

	if (allocationCallbacks)
	{
		delete allocationCallbacks;
		allocationCallbacks = nullptr;
	}
}

void CVkRenderDevice::create(const FRenderContextCreateInfo& context_ci)
{
	// Register dispatcher
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	// Create allocation callbacks
	registerAllocationCallbacks();

	check_api_result(createInstance(context_ci));
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkInstance);
	createDebugMessanger(context_ci);

	check_api_result(createSurface(context_ci));
	check_api_result(selectPhysicalDevice(context_ci));
	prepareRequestedLogicalDeviceFeatures();

	check_api_result(createLogicalDevice(context_ci));
	VULKAN_HPP_DEFAULT_DISPATCHER.init(logicalDevice.handle);

	createMemoryAllocator(context_ci);

	check_api_result(createPipelineCache(context_ci));
	check_api_result(createSwapchain(context_ci.initialWidth, context_ci.initialHeight));
}

FRenderApiErrorInfo CVkRenderDevice::createCommandPool(const FVkCommandPoolCreateInfo& command_pool_ci, FVkCommandPool& command_pool)
{
	auto& physical_device = getPhysicalDevice();

	const bool async_compute_support = physical_device.compatFlags & PHYSICAL_DEVICE_COMPAT_ASYNC_COMPUTE_FLAG_BIT;
	const bool async_transfer_support = physical_device.compatFlags & PHYSICAL_DEVICE_COMPAT_ASYNC_TRANSFER_FLAG_BIT;

	const bool is_compute_queue = (command_pool_ci.mask & QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT) && async_compute_support;
	const bool is_transfer_queue = (command_pool_ci.mask & QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT) && async_transfer_support;

	command_pool.threadId = command_pool_ci.threadId;

	vk::CommandPoolCreateInfo pool_ci{};
	pool_ci.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	uint32_t queue_family_index{ physical_device.queueFamilies.queueToFamily[QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT] };
	if (is_compute_queue)
		queue_family_index = physical_device.queueFamilies.queueToFamily[QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT];
	else if(is_transfer_queue)
		queue_family_index = physical_device.queueFamilies.queueToFamily[QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT];

	pool_ci.queueFamilyIndex = queue_family_index;

	vk::Result res = logicalDevice.handle.createCommandPool(&pool_ci, allocationCallbacks, &command_pool.handle);
#ifndef BUILD_WITHOUT_DEBUGGIND
	std::stringstream ss{};
	ss << command_pool_ci.threadId;
	debugSetObjectName(command_pool.handle, std::format("{} thread_id {}", command_pool_ci.name, ss.str()));
#endif

	return make_api_error(command_pool.handle.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyCommandPool(FVkCommandPool& command_pool)
{
	logicalDevice.handle.destroyCommandPool(command_pool.handle, allocationCallbacks);
}

void CVkRenderDevice::destroyUnusedCommandPools()
{
	for (auto& [queue_type, pools] : commandPools)
	{
		for (auto it = pools.begin(); it != pools.end();)
		{
			if (it->second.use_count() < 1)
			{
				destroyCommandPool(it->second);
				it = pools.erase(it);
				continue;
			}
			++it;
		}
	}
}

void CVkRenderDevice::destroyCommandPools()
{
	for (auto& [queue_type, pools] : commandPools)
	{
		for (auto& [thread_id, pool] : pools)
			destroyCommandPool(pool);
	}

	commandPools.clear();
}

FRenderApiErrorInfo CVkRenderDevice::createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, FVkCommandBuffer& command_buffer)
{
	auto& command_pool = getCommandPool(command_buffer_ci.mask);

	command_buffer.mask = command_buffer_ci.mask;
	command_buffer.threadId = command_pool.threadId;

	vk::CommandBufferAllocateInfo command_buffer_ai{};
	command_buffer_ai.commandPool = command_pool.handle;
	command_buffer_ai.level = command_buffer_ci.level;
	command_buffer_ai.commandBufferCount = command_buffer_ci.count;

	command_buffer.handles.resize(command_buffer_ci.count);

	vk::Result res = logicalDevice.handle.allocateCommandBuffers(&command_buffer_ai, command_buffer.handles.data());
#ifndef BUILD_WITHOUT_DEBUGGIND
	for(size_t idx = 0ull; idx < command_buffer.handles.size(); ++idx)
		debugSetObjectName(command_buffer.get(), std::format("{} #{}", command_buffer_ci.name, idx));
#endif

	return make_api_error(vk::ObjectType::eCommandBuffer, res, "Failed to allocate");
}

void CVkRenderDevice::destroyCommandBuffer(FVkCommandBuffer& command_buffer)
{
	auto& command_pool = getCommandPool(command_buffer.mask, command_buffer.threadId);
	logicalDevice.handle.freeCommandBuffers(command_pool.handle, command_buffer.handles);
}

void CVkRenderDevice::beginCommandBuffer(FVkCommandBuffer& command_buffer, vk::CommandBufferUsageFlags usage, uint32_t image_index)
{
	if (command_buffer.isRunning)
		return;

	command_buffer.commandBufferIndex = image_index;

	vk::CommandBufferBeginInfo begin_info{};
	begin_info.flags = usage;

	auto& current_command_buffer_handle = command_buffer.get();
	current_command_buffer_handle.begin(begin_info);
	command_buffer.isRunning = true;
}

void CVkRenderDevice::endCommandBuffer(FVkCommandBuffer& command_buffer)
{
	if (!command_buffer.isRunning)
		return;

	auto& current_command_buffer_handle = command_buffer.get();
	current_command_buffer_handle.end();

	command_buffer.isRunning = false;
}

FRenderApiErrorInfo CVkRenderDevice::createImage(const FVkImageCreateInfo& image_ci, FVkImage& image)
{
	vma::AllocationCreateInfo alloc_ci{};
	alloc_ci.usage = image_ci.memoryUsage;
	
	vk::Result res = memoryAllocator.createImage(&image_ci.ci, &alloc_ci, &image.handle, &image.memory, nullptr);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(image.handle, image_ci.name);
#endif
	return make_api_error(image.handle.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyImage(FVkImage& image)
{
	memoryAllocator.destroyImage(image.handle, image.memory);
}

FRenderApiErrorInfo CVkRenderDevice::createImageView(const FVkImageViewCreateInfo& image_view_ci, vk::ImageView& image_view)
{
	vk::Result res = logicalDevice.handle.createImageView(&image_view_ci.ci, allocationCallbacks, &image_view);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(image_view, image_view_ci.name);
#endif
	return make_api_error(image_view.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyImageView(vk::ImageView& image_view)
{
	logicalDevice.handle.destroyImageView(image_view, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createSampler(const FVkSamplerCreateInfo& sampler_ci, vk::Sampler& sampler)
{
	auto& physical_device = getPhysicalDevice();

	vk::SamplerCreateInfo ci{};
	ci.magFilter = sampler_ci.magFilter;
	ci.minFilter = sampler_ci.minFilter;
	ci.addressModeU = sampler_ci.addressMode;
	ci.addressModeV = sampler_ci.addressMode;
	ci.addressModeW = sampler_ci.addressMode;
	ci.borderColor = vk::BorderColor::eIntOpaqueWhite;
	ci.unnormalizedCoordinates = VK_FALSE;

	// Anisotropy filtering
	ci.anisotropyEnable = sampler_ci.enableAnisotropy;
	ci.maxAnisotropy = physical_device.properties.limits.maxSamplerAnisotropy;

	// Depth
	ci.compareEnable = sampler_ci.enableCompareOp;
	ci.compareOp = vk::CompareOp::eLess;

	ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
	ci.mipLodBias = 0.f;
	ci.minLod = 0.f;
	ci.maxLod = static_cast<float>(sampler_ci.mipLevels);

	vk::Result res = logicalDevice.handle.createSampler(&ci, allocationCallbacks, &sampler);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(sampler, sampler_ci.name);
#endif
	return make_api_error(sampler.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroySampler(vk::Sampler& sampler)
{
	logicalDevice.handle.destroySampler(sampler, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createBuffer(const FVkBufferCreateInfo& buffer_ci, FVkBuffer& buffer)
{
	auto alligned_instance_size = get_alligned_instance_size(buffer_ci.instanceSize, buffer_ci.minOffsetAlignment);

	vk::BufferCreateInfo ci{};
	buffer.instanceCount = buffer_ci.instanceCount;
	buffer.size = ci.size = alligned_instance_size * buffer_ci.instanceCount;
	ci.usage = buffer_ci.usageFlags;
	ci.sharingMode = vk::SharingMode::eExclusive;

	vma::AllocationCreateInfo alloc_ci{};
	alloc_ci.usage = buffer_ci.memoryUsage;

	vk::Result res = memoryAllocator.createBuffer(&ci, &alloc_ci, &buffer.handle, &buffer.memory, nullptr);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(buffer.handle, buffer_ci.name);
#endif
	return make_api_error(buffer.handle.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyBuffer(FVkBuffer& buffer)
{
	unmapBufferMemory(buffer);
	memoryAllocator.destroyBuffer(buffer.handle, buffer.memory);
}

FRenderApiErrorInfo CVkRenderDevice::mapBufferMemory(FVkBuffer& buffer, void** mappedMemory)
{
	if (!buffer.isMapped)
	{
		vk::Result res = memoryAllocator.mapMemory(buffer.memory, mappedMemory);
		buffer.isMapped = res == vk::Result::eSuccess;
		return make_api_error(buffer.handle.objectType, res, "Failed to map buffer memory");
	}

	return make_api_error(vk::ObjectType::eBuffer, vk::Result::eSuccess);
}

void CVkRenderDevice::unmapBufferMemory(FVkBuffer& buffer)
{
	if (buffer.isMapped)
	{
		memoryAllocator.unmapMemory(buffer.memory);
		buffer.isMapped = false;
	}
}

void CVkRenderDevice::flushBufferMemory(const FVkBuffer& buffer, vk::DeviceSize size, vk::DeviceSize offset)
{
	memoryAllocator.flushAllocation(buffer.memory, size, offset);
}

FRenderApiErrorInfo CVkRenderDevice::createDescriptorSetLayout(const FVkDescriptorSetLayoutCreateInfo& layout_ci, vk::DescriptorSetLayout& descriptor_set_layout)
{
	vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
	descriptor_set_layout_ci.bindingCount = static_cast<uint32_t>(layout_ci.layoutBindings.size());
	descriptor_set_layout_ci.pBindings = layout_ci.layoutBindings.data();
	descriptor_set_layout_ci.flags = layout_ci.flags;
	descriptor_set_layout_ci.pNext = layout_ci.next;

	vk::Result res = logicalDevice.handle.createDescriptorSetLayout(&descriptor_set_layout_ci, allocationCallbacks, &descriptor_set_layout);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(descriptor_set_layout, layout_ci.name);
#endif
	return make_api_error(descriptor_set_layout.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyDescriptorSetLayout(vk::DescriptorSetLayout& descriptor_set_layout)
{
	logicalDevice.handle.destroyDescriptorSetLayout(descriptor_set_layout);
}

FRenderApiErrorInfo CVkRenderDevice::createDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool)
{
	std::vector<vk::DescriptorPoolSize> pool_sizes{ pool_ci.poolSizes };

	// Calculate total descriptor set count in descriptor pool and update descriptor sizes
	uint32_t maxSets{ 1u };
	for (auto& pool_size : pool_sizes)
	{
		pool_size.descriptorCount *= pool_ci.usageCount;
		maxSets += pool_size.descriptorCount;
	}

	vk::DescriptorPoolCreateInfo descriptor_pool_ci{};
	descriptor_pool_ci.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	descriptor_pool_ci.pPoolSizes = pool_sizes.data();
	descriptor_pool_ci.maxSets = maxSets;
	descriptor_pool_ci.flags = pool_ci.flags;

	vk::Result res = logicalDevice.handle.createDescriptorPool(&descriptor_pool_ci, allocationCallbacks, &descriptor_pool.handle);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(descriptor_pool.handle, pool_ci.name);
#endif

	descriptor_pool.thread_id = std::this_thread::get_id();

	return make_api_error(descriptor_pool.handle.objectType, res, "Failed to create");
}

FRenderApiErrorInfo CVkRenderDevice::growDescriptorPool(const FVkDescriptorPoolCreateInfo& pool_ci, FVkDescriptorPool& descriptor_pool)
{
	destroyDescriptorPool(descriptor_pool);
	return createDescriptorPool(pool_ci, descriptor_pool);
}

void CVkRenderDevice::destroyDescriptorPool(FVkDescriptorPool& descriptor_pool)
{
	logicalDevice.handle.destroyDescriptorPool(descriptor_pool.handle, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createDescriptorSet(const FVkDescriptorSetCreateInfo& set_ci, FVkDescriptorSet& descriptor_set)
{
	std::vector<vk::DescriptorSetLayout> per_frame_layout(set_ci.descriptorCount, set_ci.layout);

	vk::DescriptorSetAllocateInfo descriptor_ai{};
	descriptor_ai.descriptorPool = set_ci.pool;
	descriptor_ai.descriptorSetCount = static_cast<uint32_t>(per_frame_layout.size());
	descriptor_ai.pSetLayouts = per_frame_layout.data();
	descriptor_ai.pNext = set_ci.next;

	descriptor_set.handles.resize(set_ci.descriptorCount);
	vk::Result res = logicalDevice.handle.allocateDescriptorSets(&descriptor_ai, descriptor_set.handles.data());

#ifndef BUILD_WITHOUT_DEBUGGIND
	for (size_t idx = 0ull; idx < descriptor_set.handles.size(); ++idx)
		debugSetObjectName(descriptor_set.handles.at(idx), std::format("{} #{}", set_ci.name, idx));
#endif

	return make_api_error(vk::ObjectType::eDescriptorSet, res, "Failed to allocate");
}

void CVkRenderDevice::destroyDescriptorSet(const FVkDescriptorPool& descriptor_pool, FVkDescriptorSet& descriptor_set)
{
	logicalDevice.handle.freeDescriptorSets(descriptor_pool.handle, descriptor_set.handles);
}

FRenderApiErrorInfo CVkRenderDevice::createShaderModule(const FVkShaderModuleCreateInfo& shader_module_ci, vk::ShaderModule& shader_module)
{
	vk::ShaderModuleCreateInfo module_ci{};
	module_ci.codeSize = shader_module_ci.shaderCode->size() * sizeof(uint32_t);
	module_ci.pCode = shader_module_ci.shaderCode->data();

	vk::Result res = logicalDevice.handle.createShaderModule(&module_ci, allocationCallbacks, &shader_module);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(shader_module, shader_module_ci.name);
#endif
	return make_api_error(shader_module.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyShaderModule(vk::ShaderModule& shader_module)
{
	logicalDevice.handle.destroyShaderModule(shader_module, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createPipelineLayout(const FVkPipelineLayoutCreateInfo& pipeline_layout_ci, vk::PipelineLayout& pipeline_layout)
{
	vk::PipelineLayoutCreateInfo layout_ci{};
	layout_ci.setLayoutCount = static_cast<uint32_t>(pipeline_layout_ci.descriptorSetLayouts.size());
	layout_ci.pSetLayouts = pipeline_layout_ci.descriptorSetLayouts.data();
	layout_ci.pushConstantRangeCount = static_cast<uint32_t>(pipeline_layout_ci.pushConstantRanges.size());
	layout_ci.pPushConstantRanges = pipeline_layout_ci.pushConstantRanges.data();
	vk::Result res = logicalDevice.handle.createPipelineLayout(&layout_ci, allocationCallbacks, &pipeline_layout);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(pipeline_layout, pipeline_layout_ci.name);
#endif
	return make_api_error(pipeline_layout.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyPipelineLayout(vk::PipelineLayout& pipeline_layout)
{
	logicalDevice.handle.destroyPipelineLayout(pipeline_layout, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createPipeline(const FVkGraphicsPipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline)
{
	vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
	vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(pipeline_ci.vertexAttributeDescriptions.size());
	vertex_input_state.pVertexAttributeDescriptions = pipeline_ci.vertexAttributeDescriptions.data();
	vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(pipeline_ci.vertexBindingDescriptions.size());
	vertex_input_state.pVertexBindingDescriptions = pipeline_ci.vertexBindingDescriptions.data();

	std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states{};
	for (uint32_t idx = 0u; idx < pipeline_ci.colorAttachmentFormats.size(); ++idx)
	{
		auto& state = color_blend_attachment_states.emplace_back();

		// TODO: generate write mask from format type
		state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		state.blendEnable = false; // TODO: add color blending support

		state.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		state.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		state.colorBlendOp = vk::BlendOp::eAdd;
		state.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		state.alphaBlendOp = vk::BlendOp::eAdd;
	}

	vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.topology = pipeline_ci.primitiveTopology;
	input_assembly.primitiveRestartEnable = false;

	vk::PipelineRasterizationStateCreateInfo rasterization_state{};
	rasterization_state.depthClampEnable = false;
	rasterization_state.rasterizerDiscardEnable = false;
	rasterization_state.polygonMode = vk::PolygonMode::eFill;
	rasterization_state.lineWidth = 1.0f;
	rasterization_state.cullMode = pipeline_ci.cullMode;
	rasterization_state.frontFace = pipeline_ci.frontFace;
	rasterization_state.depthBiasEnable = false;

	vk::PipelineMultisampleStateCreateInfo multisample_state{};
	multisample_state.rasterizationSamples = pipeline_ci.sampleCount;

	vk::PipelineColorBlendStateCreateInfo color_blending_state{};
	color_blending_state.logicOpEnable = false;
	color_blending_state.logicOp = vk::LogicOp::eCopy;
	color_blending_state.attachmentCount = static_cast<uint32_t>(color_blend_attachment_states.size());
	color_blending_state.pAttachments = color_blend_attachment_states.data();

	vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
	depth_stencil_state.depthTestEnable = pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined;
	depth_stencil_state.depthWriteEnable = pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined; // TODO: depth write should be set by texture state
	depth_stencil_state.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depth_stencil_state.back.compareOp = vk::CompareOp::eAlways;
	depth_stencil_state.depthBoundsTestEnable = false;
	depth_stencil_state.stencilTestEnable = false; // TODO: implement stencil test

	std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.pDynamicStates = dynamic_states.data();
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());

	vk::PipelineViewportStateCreateInfo viewport_state{};
	viewport_state.viewportCount = 1u;
	viewport_state.scissorCount = 1u;

	vk::PipelineTessellationStateCreateInfo tessellation_state{};
	tessellation_state.patchControlPoints = pipeline_ci.patchControlPoints;

	auto enableTessellation = pipeline_ci.primitiveTopology == vk::PrimitiveTopology::ePatchList;

	// VK_KHR_dynamic_rendering
	vk::PipelineRenderingCreateInfoKHR rendering_ci{};
	rendering_ci.colorAttachmentCount = static_cast<uint32_t>(pipeline_ci.colorAttachmentFormats.size());
	rendering_ci.pColorAttachmentFormats = pipeline_ci.colorAttachmentFormats.data();
	if (pipeline_ci.depthAttachmentFormat != vk::Format::eUndefined)
	{
		rendering_ci.depthAttachmentFormat = pipeline_ci.depthAttachmentFormat;
		if (pipeline_ci.stencilAttachmentFormat != pipeline_ci.depthAttachmentFormat && pipeline_ci.stencilAttachmentFormat != vk::Format::eUndefined)
			rendering_ci.stencilAttachmentFormat = pipeline_ci.stencilAttachmentFormat;
		else if(!is_depth_only_format(pipeline_ci.depthAttachmentFormat))
			rendering_ci.stencilAttachmentFormat = pipeline_ci.depthAttachmentFormat;
	}

	vk::GraphicsPipelineCreateInfo graphics_pipeline_ci{};
	graphics_pipeline_ci.stageCount = static_cast<uint32_t>(pipeline_ci.shaderStages.size());
	graphics_pipeline_ci.pStages = pipeline_ci.shaderStages.data();
	graphics_pipeline_ci.pVertexInputState = &vertex_input_state;
	graphics_pipeline_ci.pInputAssemblyState = &input_assembly;
	graphics_pipeline_ci.pViewportState = &viewport_state;
	graphics_pipeline_ci.pRasterizationState = &rasterization_state;
	graphics_pipeline_ci.pMultisampleState = &multisample_state;
	graphics_pipeline_ci.pColorBlendState = &color_blending_state;
	graphics_pipeline_ci.pDepthStencilState = &depth_stencil_state;
	graphics_pipeline_ci.layout = pipeline_ci.layout;
	graphics_pipeline_ci.renderPass = nullptr;
	graphics_pipeline_ci.subpass = 0;
	graphics_pipeline_ci.basePipelineHandle = nullptr;
	graphics_pipeline_ci.pDynamicState = &dynamic_state;
	graphics_pipeline_ci.pTessellationState = (enableTessellation ? &tessellation_state : nullptr);
	graphics_pipeline_ci.pNext = &rendering_ci;

	vk::Result res = logicalDevice.handle.createGraphicsPipelines(pipelineCache, 1u, &graphics_pipeline_ci, allocationCallbacks, &pipeline);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(pipeline, pipeline_ci.name);
#endif
	return make_api_error(pipeline.objectType, res, "Failed to create");
}

FRenderApiErrorInfo CVkRenderDevice::createPipeline(const FVkComputePipelineCreateInfo& pipeline_ci, vk::Pipeline& pipeline)
{
	vk::ComputePipelineCreateInfo compute_pipeline_ci{};
	compute_pipeline_ci.stage = pipeline_ci.shaderStage;
	compute_pipeline_ci.layout = pipeline_ci.layout;
	compute_pipeline_ci.basePipelineHandle = nullptr;
	compute_pipeline_ci.basePipelineIndex = -1;

	vk::Result res = logicalDevice.handle.createComputePipelines(pipelineCache, 1u, &compute_pipeline_ci, allocationCallbacks, &pipeline);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(pipeline, pipeline_ci.name);
#endif
	return make_api_error(pipeline.objectType, res, "Failed to create");
}

void CVkRenderDevice::destroyPipeline(vk::Pipeline& pipeline)
{
	logicalDevice.handle.destroyPipeline(pipeline, allocationCallbacks);
}

FRenderApiErrorInfo CVkRenderDevice::createSemaphore(const FVkSemaphoreCreateInfo& semaphore_ci, vk::Semaphore& semaphore)
{
	vk::Result res = logicalDevice.handle.createSemaphore(&semaphore_ci.ci, allocationCallbacks, &semaphore);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(semaphore, semaphore_ci.name);
#endif
	return make_api_error(semaphore.objectType, res, "Failed to create");
}

FRenderApiErrorInfo CVkRenderDevice::createFence(const FVkFenceCreateInfo& fence_ci, vk::Fence& fence)
{
	vk::Result res = logicalDevice.handle.createFence(&fence_ci.ci, allocationCallbacks, &fence);
#ifndef BUILD_WITHOUT_DEBUGGIND
	debugSetObjectName(fence, fence_ci.name);
#endif
	return make_api_error(fence.objectType, res, "Failed to create");
}

void CVkRenderDevice::submitCommandBuffer(const FVkCommandBuffer& command_buffer)
{
	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = static_cast<uint32_t>(command_buffer.handles.size());
	submitInfo.pCommandBuffers = command_buffer.handles.data();

	FVkFenceCreateInfo fence_ci{};
	vk::Fence fence{};
	createFence(fence_ci, fence);

	logicalDevice.handle.resetFences(1, &fence);

	auto& queue = getQueue(command_buffer.mask);
	queue.submit(submitInfo, fence);
	logicalDevice.handle.waitForFences(1, &fence, VK_TRUE, (std::numeric_limits<uint64_t>::max)());
	logicalDevice.handle.destroyFence(fence, allocationCallbacks);
}

vk::Result CVkRenderDevice::present(FVkCommandBuffer& command_buffer)
{
	vk::Result res = logicalDevice.handle.resetFences(1, &swapchain.inFlightFences.at(swapchain.currentFrame));
	check_api_result(make_api_error(vk::ObjectType::eFence, res, "Failed to reset"));

	try
	{
		auto& queue = getQueue(command_buffer.mask);

		vk::SubmitInfo2 submit_info{};

		vk::SemaphoreSubmitInfo semaphore_si{};
		semaphore_si.semaphore = swapchain.imageAvaliableSemaphores[swapchain.currentFrame];
		semaphore_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

		submit_info.pWaitSemaphoreInfos = &semaphore_si;
		submit_info.waitSemaphoreInfoCount = 1;

		vk::SemaphoreSubmitInfo signal_si{};
		signal_si.semaphore = swapchain.renderFinishedSemaphores[swapchain.currentFrame];
		signal_si.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

		submit_info.pSignalSemaphoreInfos = &signal_si;
		submit_info.signalSemaphoreInfoCount = 1;

		auto& current_command_buffer = command_buffer.get();

		vk::CommandBufferSubmitInfo command_buffer_si{};
		command_buffer_si.commandBuffer = current_command_buffer;

		submit_info.pCommandBufferInfos = &command_buffer_si;
		submit_info.commandBufferInfoCount = 1;

		queue.submit2(submit_info, swapchain.inFlightFences.at(swapchain.currentFrame));
	}
	catch (vk::SystemError error)
	{
		log_error("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR present_info = {};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &swapchain.renderFinishedSemaphores[swapchain.currentFrame];

	vk::SwapchainKHR swap_chains[] = { swapchain.handle };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &swapchain.currentFrame;

	auto& present_queue = getQueue(QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);
	res = present_queue.presentKHR(present_info);
	check_api_result(make_api_error(vk::ObjectType::eQueue, res, "Failed to present"));

	res = logicalDevice.handle.waitForFences(1, &swapchain.inFlightFences[swapchain.currentFrame], VK_TRUE, (std::numeric_limits<uint64_t>::max)());
	swapchain.currentFrame = (swapchain.currentFrame + 1) % swapchain.framesInFlight;

	return res;
}

vk::Result CVkRenderDevice::acquireNextImage(uint32_t& image_index)
{
	return logicalDevice.handle.acquireNextImageKHR(swapchain.handle, (std::numeric_limits<uint64_t>::max)(), swapchain.imageAvaliableSemaphores[swapchain.currentFrame], nullptr, &image_index);
}

void CVkRenderDevice::debugBeginLabel(vk::CommandBuffer& command_buffer, const std::string& name, uint32_t color)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer.beginDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
}

void CVkRenderDevice::debugInsertLabel(vk::CommandBuffer& command_buffer, const std::string& name, uint32_t color)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer.insertDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
}

void CVkRenderDevice::endLabel(vk::CommandBuffer& command_buffer)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer.endDebugUtilsLabelEXT();
#endif
}

void CVkRenderDevice::debugBeginLabel(vk::Queue& queue, const std::string& name, uint32_t color)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	queue.beginDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
}

void CVkRenderDevice::debugInsertLabel(vk::Queue& queue, const std::string& name, uint32_t color)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	queue.insertDebugUtilsLabelEXT(make_debug_label(name.c_str(), color));
#endif
}

void CVkRenderDevice::endLabel(vk::Queue& queue)
{
#ifndef BUILD_WITHOUT_DEBUGGIND
	queue.endDebugUtilsLabelEXT();
#endif
}

vk::Format CVkRenderDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	auto& phusical_device = getPhysicalDevice();

	for (const auto& candidate : candidates)
	{
		vk::FormatProperties format_properties;
		phusical_device.handle.getFormatProperties(candidate, &format_properties);

		if (tiling == vk::ImageTiling::eLinear && (format_properties.linearTilingFeatures & features) == features)
			return candidate;
		else if (tiling == vk::ImageTiling::eOptimal && (format_properties.optimalTilingFeatures & features) == features)
			return candidate;
	}

	return vk::Format::eUndefined;
}

const vk::DynamicLoader& CVkRenderDevice::getDynamicLoader() const
{
	return dynamicLoader;
}

const vk::AllocationCallbacks* CVkRenderDevice::getAllocationCallbacks() const
{
	return allocationCallbacks;
}

const vk::Instance& CVkRenderDevice::getInstance() const
{
	return vkInstance;
}

const vk::DebugUtilsMessengerEXT& CVkRenderDevice::getDebugMessanger() const
{
	return debugMessenger;
}

const vk::DebugReportCallbackEXT& CVkRenderDevice::getDebugReport() const
{
	return debugReport;
}

const vk::SurfaceKHR& CVkRenderDevice::getSurface() const
{
	return vkSurface;
}

FVkPhysicalDevice& CVkRenderDevice::getPhysicalDevice()
{
	return physicalDevices.at(selectedPhysicalDevice);
}

const vk::SampleCountFlagBits CVkRenderDevice::getMultisamplingCount() const
{
	return requiredSampleCount;
}

vk::Device& CVkRenderDevice::getLogicalDevice()
{
	return logicalDevice.handle;
}

vk::Queue& CVkRenderDevice::getQueue(uint32_t flag)
{
	auto& physical_device = getPhysicalDevice();

	const bool async_compute_support = physical_device.compatFlags & PHYSICAL_DEVICE_COMPAT_ASYNC_COMPUTE_FLAG_BIT;
	const bool async_transfer_support = physical_device.compatFlags & PHYSICAL_DEVICE_COMPAT_ASYNC_TRANSFER_FLAG_BIT;

	const bool is_present_queue = (flag & QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);
	const bool is_compute_queue = (flag & QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT) && async_compute_support;
	const bool is_transfer_queue = (flag & QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT) && async_transfer_support;

	// Code can require compute or transfer queue, but device might not support it. It means that we will use graphics as default.
	uint32_t selected_queue_type{ QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT };
	if (is_present_queue)
		selected_queue_type = QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT;
	else if (is_compute_queue)
		selected_queue_type = QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
	else if (is_transfer_queue)
		selected_queue_type = QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT;
	
	return logicalDevice.queues[selected_queue_type];
}

vma::Allocator& CVkRenderDevice::getMemoryAllocator()
{
	return memoryAllocator;
}

const vk::PipelineCache& CVkRenderDevice::getPipelineCache() const
{
	return pipelineCache;
}

std::vector<char> CVkRenderDevice::getPipelineCacheData() const
{
	std::vector<char> pipeline_cache_data{};

	size_t data_size{ 0ull };
	void* data_ptr{ nullptr };
	vk::Result res = logicalDevice.handle.getPipelineCacheData(pipelineCache, &data_size, data_ptr);

	if (res == vk::Result::eSuccess)
	{
		pipeline_cache_data.resize(data_size);
		std::memcpy(pipeline_cache_data.data(), data_ptr, data_size);
	}

	return pipeline_cache_data;
}

const FSwapchain& CVkRenderDevice::getSwapchain() const
{
	return swapchain;
}

const FVkCommandPool& CVkRenderDevice::getCommandPool(uint32_t mask, const std::thread::id& thread_id)
{
	auto found_it = commandPools.find(mask);
	if (found_it != commandPools.end())
	{
		auto thread_it = found_it->second.find(thread_id);
		if (thread_it != found_it->second.end())
			return thread_it->second;
	}

	auto& new_pool = commandPools[mask];

	FVkCommandPoolCreateInfo pool_ci{};
	pool_ci.mask = mask;
	pool_ci.threadId = thread_id;

	auto& pool = new_pool[thread_id];
	check_api_result(createCommandPool(pool_ci, pool));
	return new_pool[thread_id];
}

void CVkRenderDevice::registerAllocationCallbacks()
{
	allocationCallbacks = new vk::AllocationCallbacks();
	memset(allocationCallbacks, 0, sizeof(vk::AllocationCallbacks));

	allocationCallbacks->pfnAllocation = (PFN_vkAllocationFunction)(&vk_malloc);
	allocationCallbacks->pfnReallocation = (PFN_vkReallocationFunction)(&vk_realloc);
	allocationCallbacks->pfnFree = (PFN_vkFreeFunction)(&vk_free);
	allocationCallbacks->pfnInternalAllocation = (PFN_vkInternalAllocationNotification)(&vk_malloc_notification);
	allocationCallbacks->pfnInternalFree = (PFN_vkInternalFreeNotification)(&vk_free_notification);
}

FRenderApiErrorInfo CVkRenderDevice::createInstance(const FRenderContextCreateInfo& context_ci)
{
	// Load instance layer props
	auto layer_props = vk::enumerateInstanceLayerProperties();
	for (auto& layer : layer_props)
		avaliableLayers.push_back(layer.layerName);

	if (context_ci.validationState && !check_contains_all(context_ci.validationLayers, avaliableLayers))
		return make_api_error(vk::ObjectType::eInstance, vk::Result::eErrorUnknown, "Some validation layers is not supported");

	vk::ApplicationInfo app_info{};
	app_info.applicationVersion = app_info.engineVersion = app_info.apiVersion = get_api_version(context_ci.apiVersion);
	app_info.pApplicationName = app_info.pEngineName = context_ci.appName.c_str();

	auto instance_extensions = get_instance_extension_names();

	vk::InstanceCreateInfo instance_ci{};
	instance_ci.pApplicationInfo = &app_info;
	instance_ci.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	instance_ci.ppEnabledExtensionNames = instance_extensions.data();

	// Apply validation features
	std::vector<vk::ValidationFeatureEnableEXT> feature_enables{};
	vk::ValidationFeaturesEXT validation_features{};

	if (context_ci.validationFeatures & GPU_ASSISTED_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);
	if (context_ci.validationFeatures & GPU_ASSISTED_RESERVE_BINDING_SLOT_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot);
	if (context_ci.validationFeatures & BEST_PRACTICE_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eBestPractices);
	if (context_ci.validationFeatures & DEBUG_PRINTF_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
	if (context_ci.validationFeatures & SYNCHRONIZATION_VALIDATION_FLAG_BIT)
		feature_enables.emplace_back(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);

	validation_features.enabledValidationFeatureCount = static_cast<uint32_t>(feature_enables.size());
	validation_features.pEnabledValidationFeatures = feature_enables.data();

	std::vector<const char*> layer_names{};
	for (auto& layer_name : context_ci.validationLayers)
		layer_names.emplace_back(layer_name.c_str());

	if (context_ci.validationState)
	{
		instance_ci.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		instance_ci.ppEnabledLayerNames = layer_names.data();
		instance_ci.pNext = &validation_features;
	}
	return make_api_error(vk::ObjectType::eInstance, vk::createInstance(&instance_ci, allocationCallbacks, &vkInstance), "Failed to create");
}

void CVkRenderDevice::createDebugMessanger(const FRenderContextCreateInfo& context_ci)
{
	if (context_ci.validationState)
	{
		vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_ci{};
		debug_messenger_ci.flags = vk::DebugUtilsMessengerCreateFlagsEXT();
		debug_messenger_ci.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		debug_messenger_ci.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debug_messenger_ci.pfnUserCallback = &debug_messenger_callback;
		debug_messenger_ci.pUserData = this;

		debugMessenger = vkInstance.createDebugUtilsMessengerEXT(debug_messenger_ci, allocationCallbacks);

		vk::DebugReportCallbackCreateInfoEXT debug_report_ci{};
		debug_report_ci.flags = vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError;
		debug_report_ci.pfnCallback = &debug_report_callback;
		debug_report_ci.pUserData = this;

		debugReport = vkInstance.createDebugReportCallbackEXT(debug_report_ci, allocationCallbacks);
	}
}

FRenderApiErrorInfo CVkRenderDevice::createSurface(const FRenderContextCreateInfo& context_ci)
{
	vk::Result result{ vk::Result::eErrorUnknown };
#if defined( VK_USE_PLATFORM_WIN32_KHR )
	vk::Win32SurfaceCreateInfoKHR surface_ci{};
	surface_ci.hinstance = context_ci.hinstance;
	surface_ci.hwnd = context_ci.hwnd;
	
	result = vkInstance.createWin32SurfaceKHR(&surface_ci, allocationCallbacks, &vkSurface);
#else
#endif;
	return make_api_error(vk::ObjectType::eSurfaceKHR, result, "Failed to create");
}

void CVkRenderDevice::selectQueueFamilies(FVkPhysicalDevice& device_info)
{
	std::unordered_map<uint32_t, vk::QueueFamilyProperties> family_properties{};

	auto& physical_device = device_info.handle;
	auto& families_info = device_info.queueFamilies;
	auto& family_usages = families_info.familyUsages;

	{
		auto family_props = physical_device.getQueueFamilyProperties();
		for (uint32_t index = 0u; index < family_props.size(); ++index)
			family_properties[index] = family_props.at(index);
	}

	auto is_family_queues_enought = [&family_usages](uint32_t index, const vk::QueueFamilyProperties& prop) -> bool { return family_usages[index] < prop.queueCount; };
	auto is_family_selected = [&families_info](uint32_t mask) -> bool { return families_info.queueMask & mask; };
	auto apply_family_usage = [&families_info](uint32_t index, uint32_t mask) -> void { families_info.queueToFamily[mask] = index; families_info.queueMask |= mask; families_info.familyUsages[index]++; };
	
	for (auto& [index, prop] : family_properties)
	{
		const bool has_graphics_bit = static_cast<bool>(prop.queueFlags & vk::QueueFlagBits::eGraphics);
		const bool has_compute_bit = static_cast<bool>(prop.queueFlags & vk::QueueFlagBits::eCompute);
		const bool has_transfer_bit = static_cast<bool>(prop.queueFlags & vk::QueueFlagBits::eTransfer);
		const bool has_present_bit = physical_device.getSurfaceSupportKHR(index, vkSurface);

		const bool is_present_only = has_present_bit && !(has_graphics_bit || has_compute_bit || has_transfer_bit);
		const bool is_compute_compat = has_compute_bit && !has_graphics_bit;
		const bool is_transfer_only = has_transfer_bit && !has_compute_bit && !has_graphics_bit;

		// Looking for present family
		if (is_present_only && !is_family_selected(QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);

		// Looking for suitable graphics family
		if (has_graphics_bit && !is_family_selected(QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT);

		// Looking for compute only family
		if (is_compute_compat && !is_family_selected(QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT);

		// Looking for transfer only family
		if (is_transfer_only && !is_family_selected(QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT);
	}

	// Try to find compute and transfer family if not found
	for (auto& [index, prop] : family_properties)
	{
		const bool has_compute_bit = static_cast<bool>(prop.queueFlags & vk::QueueFlagBits::eCompute);
		const bool has_transfer_bit = static_cast<bool>(prop.queueFlags & vk::QueueFlagBits::eTransfer);
		const bool has_present_bit = physical_device.getSurfaceSupportKHR(index, vkSurface);

		if (has_present_bit && !is_family_selected(QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);

		if (has_compute_bit && !is_family_selected(QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT);

		if (has_transfer_bit && !is_family_selected(QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT) && is_family_queues_enought(index, prop))
			apply_family_usage(index, QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT);
	}

	if ((families_info.queueMask & QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT) && (families_info.queueMask & QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT))
		device_info.compatFlags |= PHYSICAL_DEVICE_COMPAT_BETTER_THEN_NOTHING_FLAG_BIT;

	if (families_info.queueMask & QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT)
		device_info.compatFlags |= PHYSICAL_DEVICE_COMPAT_ASYNC_COMPUTE_FLAG_BIT;

	if (families_info.queueMask & QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT)
		device_info.compatFlags |= PHYSICAL_DEVICE_COMPAT_ASYNC_TRANSFER_FLAG_BIT;

	std::set<uint32_t> unique_families{};
	for (auto& [index, count] : family_usages)
	{
		if(count > 0)
			unique_families.emplace(index);
	}

	device_info.queueFamilies.uniqueFamilies = std::vector<uint32_t>(unique_families.begin(), unique_families.end());
}

FRenderApiErrorInfo CVkRenderDevice::selectPhysicalDevice(const FRenderContextCreateInfo& context_ci)
{
	auto api_version = get_api_version(context_ci.apiVersion);
	auto avaliable_devices = vkInstance.enumeratePhysicalDevices();

	int32_t fallback_device_index{ -1 };
	for (auto& device : avaliable_devices)
	{
		auto device_index = physicalDevices.size();
		physicalDevices.emplace_back(make_physical_device(device));
		auto& current_device = physicalDevices.back();

		if (current_device.properties.apiVersion < api_version)
			continue;

		if (!check_contains_all(context_ci.requiredDeviceExtensions, current_device.avaliableExtensions))
			continue;

		// Get supported queue families for this device
		selectQueueFamilies(current_device);
		const bool is_device_better_then_nothing = current_device.compatFlags & PHYSICAL_DEVICE_COMPAT_BETTER_THEN_NOTHING_FLAG_BIT;
		const bool is_device_best_candidate = current_device.compatFlags & PHYSICAL_DEVICE_COMPAT_BEST_CANDIDATE_FLAG_BIT;
		if (!(is_device_best_candidate || is_device_better_then_nothing))
			continue;

		// Discrete gpu is in higher priority
		if (current_device.properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
		{
			fallback_device_index = device_index;
			continue;
		}

		selectedPhysicalDevice = device_index;
		break;
	}

	if (selectedPhysicalDevice < 0)
	{
		if (fallback_device_index < 0)
			return make_api_error(vk::ObjectType::ePhysicalDevice, vk::Result::eErrorUnknown, "Not found any suitable device");
		selectedPhysicalDevice = fallback_device_index;
	}

	auto& selected_device = getPhysicalDevice();
	for (auto& extension_name : context_ci.requiredDeviceExtensions)
		selected_device.supportedExtensions[utl::const_hash(extension_name.c_str())] = true;

	for (auto& extension_name : context_ci.optionalDeviceExtensions)
		selected_device.supportedExtensions[utl::const_hash(extension_name.c_str())] = check_contains_single(extension_name, selected_device.avaliableExtensions);

	return make_api_error(vk::ObjectType::ePhysicalDevice, vk::Result::eSuccess);
}

void CVkRenderDevice::prepareRequestedLogicalDeviceFeatures()
{
	auto& physical_device = getPhysicalDevice();
	for (auto& extension_name : physical_device.avaliableExtensions)
	{
		auto extension_name_hash = utl::const_hash(extension_name.c_str());
		if (physical_device.supportedExtensions.count(extension_name_hash))
		{
			if (extension_name == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceSynchronization2FeaturesKHR>();
			else if (extension_name == VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR>();
			else if (extension_name == VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceDynamicRenderingFeaturesKHR>();
			else if (extension_name == VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>();
			else if (extension_name == VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>();
			else if (extension_name == VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
			else if (extension_name == VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();
			else if (extension_name == VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR>();
			else if (extension_name == VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceDescriptorBufferFeaturesEXT>();
			else if (extension_name == VK_EXT_SHADER_OBJECT_EXTENSION_NAME)
				requestExtensionFeatures<vk::PhysicalDeviceShaderObjectFeaturesEXT>();
		}
	}
}

FRenderApiErrorInfo CVkRenderDevice::createLogicalDevice(const FRenderContextCreateInfo& context_ci)
{
	auto api_version = get_api_version(context_ci.apiVersion);
	auto& physical_device = getPhysicalDevice();
	bool bindless_support = physical_device.supportedExtensions.count(utl::const_hash(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME));
	bool timeline_semaphore_support = physical_device.supportedExtensions.count(utl::const_hash(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME));

	if (!is_sample_count_supported(physical_device, requiredSampleCount))
		requiredSampleCount = vk::SampleCountFlagBits::e1;

	// Get queue create infos
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};
	std::unordered_map<uint32_t, std::vector<float>> queue_priorities{};
	for (auto& [index, usages] : physical_device.queueFamilies.familyUsages)
	{
		for (size_t usage = 0; usage < usages; ++usage)
			queue_priorities[index].emplace_back(1.f);

		auto& create_info = queue_create_infos.emplace_back();
		create_info.flags = vk::DeviceQueueCreateFlags{};
		create_info.queueFamilyIndex = index;
		create_info.queueCount = usages;
		create_info.pQueuePriorities = queue_priorities[index].data();
	}

	// Vulkan 1.0 features
	vk::PhysicalDeviceFeatures vk10features{};
	vk10features.samplerAnisotropy = true;
	vk10features.sampleRateShading = true;
	vk10features.fillModeNonSolid = true;
	vk10features.multiViewport = true;
	vk10features.geometryShader = true;
	vk10features.tessellationShader = true;
	vk10features.fragmentStoresAndAtomics = true;
	vk10features.logicOp = true;
	vk10features.imageCubeArray = true;
	vk10features.depthClamp = true;
	vk10features.independentBlend = true;

	vk::DeviceCreateInfo device_ci{};
	device_ci.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	device_ci.pQueueCreateInfos = queue_create_infos.data();
	device_ci.pEnabledFeatures = &vk10features;
	device_ci.pNext = logicalDevice.lastExtensionFeature;

	std::vector<const char*> enabled_extension_names{};
	for (auto& ext_name : physical_device.avaliableExtensions)
	{
		auto ext_name_c = ext_name.c_str();
		if (physical_device.supportedExtensions.count(utl::const_hash(ext_name_c)))
			enabled_extension_names.emplace_back(ext_name_c);
	}

	device_ci.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
	device_ci.ppEnabledExtensionNames = enabled_extension_names.data();

	std::vector<const char*> layer_names{};
	for (auto& layer_name : context_ci.validationLayers)
		layer_names.emplace_back(layer_name.c_str());

	if (context_ci.validationState)
	{
		device_ci.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
		device_ci.ppEnabledLayerNames = layer_names.data();
	}

	auto res = make_api_error(vk::ObjectType::eDevice, physical_device.handle.createDevice(&device_ci, allocationCallbacks, &logicalDevice.handle), "Failed to create");

	auto get_queue_family_index = [&physical_device](uint32_t mask) -> uint32_t { return physical_device.queueFamilies.queueToFamily.count(mask) ? physical_device.queueFamilies.queueToFamily[mask] : ~0u; };

	std::unordered_map<uint32_t, uint32_t> family_usages{};
	for (auto& [type, family] : physical_device.queueFamilies.queueToFamily)
		logicalDevice.queues[type] = logicalDevice.handle.getQueue(family, family_usages[family]++);

	return res;
}

void CVkRenderDevice::createMemoryAllocator(const FRenderContextCreateInfo& context_ci)
{
	vma::VulkanFunctions vk_functions{};
	vk_functions.vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	vk_functions.vkGetDeviceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

	auto& physical_device = getPhysicalDevice();
	vma::AllocatorCreateInfo allocator_ci{};
	allocator_ci.instance = vkInstance;
	allocator_ci.physicalDevice = physical_device.handle;
	allocator_ci.device = logicalDevice.handle;
	allocator_ci.pAllocationCallbacks = allocationCallbacks;
	allocator_ci.vulkanApiVersion = get_api_version(context_ci.apiVersion);
	allocator_ci.pVulkanFunctions = &vk_functions;

	memoryAllocator = vma::createAllocator(allocator_ci);
}

FRenderApiErrorInfo CVkRenderDevice::createPipelineCache(const FRenderContextCreateInfo& context_ci)
{
	vk::PipelineCacheCreateInfo pipeline_cache_ci{};
	pipeline_cache_ci.initialDataSize = static_cast<uint32_t>(context_ci.pipelineCache.size());
	pipeline_cache_ci.pInitialData = context_ci.pipelineCache.data();
	return make_api_error(vk::ObjectType::ePipelineCache, logicalDevice.handle.createPipelineCache(&pipeline_cache_ci, allocationCallbacks, &pipelineCache), "Failed to create");
}

FRenderApiErrorInfo CVkRenderDevice::createSwapchain(uint32_t surface_width, uint32_t surface_height)
{
	auto& physical_device = getPhysicalDevice();

	vk::SwapchainKHR old_swapchain{ swapchain.handle };
	swapchain.handle = nullptr;

	auto surface_caps = physical_device.handle.getSurfaceCapabilitiesKHR(vkSurface);
	auto surface_formats = physical_device.handle.getSurfaceFormatsKHR(vkSurface);
	auto present_modes = physical_device.handle.getSurfacePresentModesKHR(vkSurface);

	auto surface_format = choice_surface_format(surface_formats);
	auto present_mode = choice_surface_present_mode(present_modes);
	swapchain.extent = choice_swapchain_extent(surface_caps, surface_width, surface_height);
	swapchain.format = surface_format.format;

	uint32_t image_count{ swapchain.framesInFlight };
	if (surface_caps.maxImageCount > 0 && image_count > surface_caps.maxImageCount)
		image_count = surface_caps.maxImageCount;
	else if (image_count < surface_caps.minImageCount)
		image_count = surface_caps.minImageCount;
	swapchain.framesInFlight = image_count;

	vk::SwapchainCreateInfoKHR swapchain_ci{};
	swapchain_ci.surface = vkSurface;
	swapchain_ci.minImageCount = image_count;
	swapchain_ci.imageFormat = surface_format.format;
	swapchain_ci.imageColorSpace = surface_format.colorSpace;
	swapchain_ci.imageExtent = swapchain.extent;
	swapchain_ci.imageArrayLayers = 1u;
	swapchain_ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	if (surface_caps.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
		swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
	if (surface_caps.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
		swapchain_ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;

	const auto graphics_family = physical_device.queueFamilies.queueToFamily[QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT];
	const auto present_family = physical_device.queueFamilies.queueToFamily[QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT];
	if (graphics_family != present_family)
	{
		swapchain_ci.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchain_ci.queueFamilyIndexCount = static_cast<uint32_t>(physical_device.queueFamilies.uniqueFamilies.size());
		swapchain_ci.pQueueFamilyIndices = physical_device.queueFamilies.uniqueFamilies.data();
	}
	else
		swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;

	swapchain_ci.preTransform = surface_caps.currentTransform;
	swapchain_ci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchain_ci.presentMode = present_mode;
	swapchain_ci.clipped = VK_TRUE;
	swapchain_ci.oldSwapchain = old_swapchain;

	vk::Result res = logicalDevice.handle.createSwapchainKHR(&swapchain_ci, allocationCallbacks, &swapchain.handle);
	check_api_result(make_api_error(vk::ObjectType::eSwapchainKHR, res, "Failed to create"));

	swapchain.images.resize(image_count);
	res = logicalDevice.handle.getSwapchainImagesKHR(swapchain.handle, &image_count, swapchain.images.data());
	check_api_result(make_api_error(vk::ObjectType::eSwapchainKHR, res, "Failed to create images"));

	if (old_swapchain)
		logicalDevice.handle.destroySwapchainKHR(old_swapchain, allocationCallbacks);

	// Create swapchain image views
	{
		swapchain.imageViews.resize(image_count);

		FVkImageViewCreateInfo image_view_ci{};
		image_view_ci.ci.viewType = vk::ImageViewType::e2D;
		image_view_ci.ci.format = swapchain.format;
		image_view_ci.ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		image_view_ci.ci.subresourceRange.baseMipLevel = 0;
		image_view_ci.ci.subresourceRange.levelCount = 1;
		image_view_ci.ci.subresourceRange.baseArrayLayer = 0;
		image_view_ci.ci.subresourceRange.layerCount = 1;
		image_view_ci.ci.components.r = vk::ComponentSwizzle::eR;
		image_view_ci.ci.components.g = vk::ComponentSwizzle::eG;
		image_view_ci.ci.components.b = vk::ComponentSwizzle::eB;
		image_view_ci.ci.components.a = vk::ComponentSwizzle::eA;

		for (uint32_t idx = 0u; idx < image_count; ++idx)
		{
#ifndef BUILD_WITHOUT_DEBUGGIND
			image_view_ci.name = std::format("frame #{}", idx);
#endif
			image_view_ci.ci.image = swapchain.images.at(idx);
			check_api_result(createImageView(image_view_ci, swapchain.imageViews.at(idx)));
		}
	}

	// Create swapchain sync objects
	{
		swapchain.imageAvaliableSemaphores.resize(image_count);
		swapchain.renderFinishedSemaphores.resize(image_count);
		swapchain.inFlightFences.resize(image_count);

		FVkSemaphoreCreateInfo ia_semaphore_ci{};
		FVkSemaphoreCreateInfo rf_semaphore_ci{};
		FVkFenceCreateInfo fence_ci{};
		fence_ci.ci.flags = vk::FenceCreateFlagBits::eSignaled;
		for (size_t idx = 0ull; idx < image_count; ++idx)
		{
#ifndef BUILD_WITHOUT_DEBUGGIND
			ia_semaphore_ci.name = std::format("image avaliable #{}", idx);
			rf_semaphore_ci.name = std::format("render finished #{}", idx);
			fence_ci.name = std::format("frame #{}", idx);
#endif
			check_api_result(createSemaphore(ia_semaphore_ci, swapchain.imageAvaliableSemaphores.at(idx)));
			check_api_result(createSemaphore(rf_semaphore_ci, swapchain.renderFinishedSemaphores.at(idx)));
			check_api_result(createFence(fence_ci, swapchain.inFlightFences.at(idx)));
		}
	}

	return make_api_error(vk::ObjectType::eSwapchainKHR, vk::Result::eSuccess);
}

void CVkRenderDevice::cleanupSwapchain()
{
	for (size_t idx = 0ull; idx < swapchain.framesInFlight; ++idx)
	{
		logicalDevice.handle.destroyFence(swapchain.inFlightFences.at(idx), allocationCallbacks);
		logicalDevice.handle.destroyImageView(swapchain.imageViews.at(idx), allocationCallbacks);
		logicalDevice.handle.destroySemaphore(swapchain.imageAvaliableSemaphores.at(idx), allocationCallbacks);
		logicalDevice.handle.destroySemaphore(swapchain.renderFinishedSemaphores.at(idx), allocationCallbacks);
	}
}
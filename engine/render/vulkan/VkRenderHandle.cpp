#include "VkRenderHandle.h"

#include <VkShaderReflection.h>

#include <logger/logger.h>

using namespace engine::render::vulkan;

template<class _Ty>
FRenderApiErrorInfo create_and_apply_shader_reflection_data(const std::unique_ptr<CVkRenderContext>& context, const _Ty& shader_ci, const FVkShaderReflectionData& reflection, std::vector<vk::PipelineShaderStageCreateInfo>& shader_stages, FVkShader& shader)
{
	// Apply specialization constants
	for (auto& shader_stage : shader_stages)
	{
		auto found = reflection.specializationConstantStages.find(shader_stage.stage);
		if (found != reflection.specializationConstantStages.end())
			shader_stage.pSpecializationInfo = &found->second.info;
	}

	// Create descriptor set layouts
	for (const auto& [set, layout_bindings] : reflection.descriptorSetLayoutBindings)
	{
		FVkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
		descriptor_set_layout_ci.name = std::format("{}: set #{}", shader_ci.name, set);
#endif
		descriptor_set_layout_ci.layoutBindings = layout_bindings; // TODO: may be send pointer to vector here?

		auto& descriptor_set_Layout = shader.descriptorSetLayouts[set];
		RETURN_WHEN_FAULT(context->createDescriptorSetLayout(descriptor_set_layout_ci, descriptor_set_Layout));
	}

	// Create pipeline layout
	FVkPipelineLayoutCreateInfo layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	layout_ci.name = shader_ci.name;
#endif
	// Send descriptor set layouts to create info
	for (const auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		layout_ci.descriptorSetLayouts.emplace_back(descriptor_set_layout);

	// Prepare push blocks info
	uint32_t current_offset{ 0u };
	for (const auto& [name, block] : reflection.pushBlocks)
	{
		layout_ci.pushConstantRanges.emplace_back();
		auto& push_constant_range = layout_ci.pushConstantRanges.back();
		push_constant_range.stageFlags = block.stageFlags;
		push_constant_range.offset = current_offset;
		push_constant_range.size = static_cast<uint32_t>(block.size);
		current_offset += push_constant_range.size;
	}

	return context->createPipelineLayout(layout_ci, shader.layout);
}

CVkRenderHandle::~CVkRenderHandle()
{

}

void CVkRenderHandle::create(const FRenderContextCreateInfo& context_ci)
{
	context = std::make_unique<CVkRenderContext>();
	context->create(context_ci);

	auto& swapchain = context->getSwapchain();

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = swapchain.framesInFlight;
	command_buffer_ci.mask = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "Graphics";
#endif

	auto result = context->createCommandBuffer(command_buffer_ci, commandBuffer);
}

void CVkRenderHandle::syncGPUIdle()
{
	auto& logical_device = context->getLogicalDevice();
	logical_device.waitIdle();
}

void CVkRenderHandle::updateSurface(int32_t surface_width, int32_t surface_height)
{
	context->cleanupSwapchain();
	context->createSwapchain(surface_width, surface_height);
	context->swapchain.currentFrame = 0u;

	this->surface_width = surface_width;
	this->surface_height = surface_height;
}

void CVkRenderHandle::beginFrameCommandBuffers()
{
	uint32_t image_index{ 0u };

	vk::Result res{};
	try { res = context->acquireNextImage(image_index); }
	catch (vk::OutOfDateKHRError error) { res = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("Failed to acquire swap chain image!"); }

	if (res == vk::Result::eErrorOutOfDateKHR)
		updateSurface(surface_width, surface_height);
	else if (res == vk::Result::eSuboptimalKHR)
		log_error("Failed to acquire swap chain image!");

	if (res != vk::Result::eSuccess)
		return;

	isFrameStarted = true;

	context->beginCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eRenderPassContinue, image_index);
}

void CVkRenderHandle::endFrameCommandBuffers()
{
	context->endCommandBuffer(commandBuffer);
	isFrameStarted = false;

	vk::Result presentResult{};
	try { presentResult = context->present(commandBuffer); }
	catch (vk::OutOfDateKHRError err) { presentResult = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("failed to present swap chain image!"); }

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		updateSurface(surface_width, surface_height);

	context->destroyUnusedCommandPools();
}

FRenderApiErrorInfo CVkRenderHandle::createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture)
{
	texture.layout = texture_ci.initialLayout;

	FVkImageCreateInfo image_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	image_ci.name = texture_ci.name;
#endif

	if (texture_ci.extent.depth > 1)
		image_ci.ci.imageType = vk::ImageType::e3D;
	else
		image_ci.ci.imageType = static_cast<vk::ImageType>(texture_ci.numDimensions - 1u);

	image_ci.ci.extent = texture_ci.extent;
	image_ci.ci.mipLevels = texture_ci.mipLevelCount;

	const bool isCubemapArray = texture_ci.isCubemap && texture_ci.isArray;
	if (isCubemapArray)
		image_ci.ci.arrayLayers = texture_ci.arrayLayersCount * 6u;
	else if (texture_ci.isArray)
		image_ci.ci.arrayLayers = texture_ci.arrayLayersCount;
	else if (texture_ci.isCubemap)
		image_ci.ci.arrayLayers = 6u;
	else
		image_ci.ci.arrayLayers = 1u;

	image_ci.memoryUsage = vma::MemoryUsage::eUnknown;
	image_ci.ci.format = texture_ci.format;
	image_ci.ci.initialLayout = texture_ci.initialLayout;
	image_ci.ci.usage = texture_ci.usageFlags;
	image_ci.ci.samples = texture_ci.sampleCount;
	image_ci.ci.sharingMode = vk::SharingMode::eExclusive;
	image_ci.ci.flags = texture_ci.isCubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(context->createImage(image_ci, texture.image));

	FVkImageViewCreateInfo image_view_ci{};
	
	if (isCubemapArray)
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	else if (texture_ci.isArray)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if(texture_ci.isCubemap)
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	else if(texture_ci.extent.depth > 1)
		image_view_ci.ci.viewType = vk::ImageViewType::e3D;
	else
		image_view_ci.ci.viewType = vk::ImageViewType::e2D;

	image_view_ci.ci.format = texture_ci.format;
	image_view_ci.ci.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	image_view_ci.ci.subresourceRange.aspectMask = texture_ci.aspectFlags;
	image_view_ci.ci.subresourceRange.baseArrayLayer = 0u;
	image_view_ci.ci.subresourceRange.layerCount = image_ci.ci.arrayLayers;
	image_view_ci.ci.image = texture.image.handle;

	texture.imageViews.resize(texture_ci.mipLevelCount);
	for (uint32_t mip = 0u; mip < texture_ci.mipLevelCount; ++mip)
	{
		image_view_ci.ci.subresourceRange.baseMipLevel = mip;
		image_view_ci.ci.subresourceRange.levelCount = mip == 0u ?texture_ci.mipLevelCount : 1u;
#ifndef BUILD_WITHOUT_DEBUGGIND
		image_view_ci.name = std::format("{} mip #{}", texture_ci.name, mip);
#endif
		RETURN_WHEN_FAULT(context->createImageView(image_view_ci, texture.imageViews.at(mip)))
	}

	FVkSamplerCreateInfo sampler_ci{};
	sampler_ci.addressMode = texture_ci.addressMode;
	sampler_ci.enableAnisotropy = texture_ci.enableAnisotropy;
	sampler_ci.enableCompareOp = texture_ci.enableCompareOp;
	sampler_ci.minFilter = sampler_ci.magFilter = texture_ci.filter;
	sampler_ci.mipLevels = texture_ci.mipLevelCount;
#ifndef BUILD_WITHOUT_DEBUGGIND
	sampler_ci.name = texture_ci.name;
#endif

	return context->createSampler(sampler_ci, texture.sampler);
}

void CVkRenderHandle::destroyTexture(FVkTexture& texture)
{
	context->destroySampler(texture.sampler);
	
	for (auto& image_view : texture.imageViews)
		context->destroyImageView(image_view);

	context->destroyImage(texture.image);
}

FRenderApiErrorInfo CVkRenderHandle::createShader(const FVkGraphicsShaderCreateInfo& shader_ci, FVkShader& shader)
{
	const char* shaderEntryPoint{ "main" };

	CVkShaderReflectionBuilder reflection_builder{};

	FVkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.primitiveTopology = shader_ci.primitiveTopology;
	pipeline_ci.cullMode = shader_ci.cullMode;
	pipeline_ci.frontFace = shader_ci.frontFace;
	pipeline_ci.depthTest = shader_ci.depthTest;
	pipeline_ci.colorAttachmentCount = shader_ci.colorAttachmentCount;
	pipeline_ci.sampleCount = shader_ci.sampleCount;
	pipeline_ci.patchControlPoints = shader_ci.patchControlPoints;
	pipeline_ci.vertexBindingDescriptions = shader_ci.vertexBindingDescriptions;
	pipeline_ci.vertexAttributeDescriptions = shader_ci.vertexAttributeDescriptions;

	// Create shader stages
	uint32_t shader_index{ 0u };
	for (const auto& [stage, code] : shader_ci.shaderStageCode)
	{
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", shader_ci.name, vk::to_string(stage), shader_index);
#endif

		shader.shaderModules.emplace_back();
		auto& shader_module = shader.shaderModules.back();
		RETURN_WHEN_FAULT(context->createShaderModule(shader_module_ci, shader_module));

		pipeline_ci.shaderStages.emplace_back();
		auto& stage_ci = pipeline_ci.shaderStages.back();
		stage_ci.stage = stage;
		stage_ci.module = shader_module;
		stage_ci.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(stage, code);
		shader_index++;
	}

	auto& shader_reflection = reflection_builder.build();

	RETURN_WHEN_FAULT(create_and_apply_shader_reflection_data(context, shader_ci, shader_reflection, pipeline_ci.shaderStages, shader));

	pipeline_ci.layout = shader.layout;

	shader.handles.emplace_back();
	auto& pipeline = shader.handles.back();

	return context->createPipeline(pipeline_ci, pipeline);
}

FRenderApiErrorInfo CVkRenderHandle::createShader(const FVkComputeShaderCreateInfo& shader_ci, FVkShader& shader)
{
	const char* shaderEntryPoint{ "main" };

	CVkShaderReflectionBuilder reflection_builder{};
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages{};

	for (size_t idx = 0ull; idx < shader_ci.shaderCode.size(); ++idx)
	{
		auto& code = shader_ci.shaderCode.at(idx);
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", shader_ci.name, vk::to_string(vk::ShaderStageFlagBits::eCompute), idx);
#endif

		shader.shaderModules.emplace_back();
		auto& shader_module = shader.shaderModules.back();
		RETURN_WHEN_FAULT(context->createShaderModule(shader_module_ci, shader_module));

		shader_stages.emplace_back();
		auto& stage = shader_stages.back();
		stage.stage = vk::ShaderStageFlagBits::eCompute;
		stage.module = shader_module;
		stage.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(vk::ShaderStageFlagBits::eCompute, code);
	}

	auto& shader_reflection = reflection_builder.build();

	RETURN_WHEN_FAULT(create_and_apply_shader_reflection_data(context, shader_ci, shader_reflection, shader_stages, shader));

	FVkComputePipelineCreateInfo pipeline_ci{};
	pipeline_ci.layout = shader.layout;

	for (size_t idx = 0ull; idx < shader_stages.size(); ++idx)
	{
		auto& stage = shader_stages.at(idx);
		pipeline_ci.shaderStage = stage;
#ifndef BUILD_WITHOUT_DEBUGGIND
		pipeline_ci.name = std::format("{} #{}", shader_ci.name, idx);
#endif

		shader.handles.emplace_back();
		auto& pipeline = shader.handles.back();

		RETURN_WHEN_FAULT(context->createPipeline(pipeline_ci, pipeline));
	}

	return make_api_error(vk::ObjectType::ePipeline, vk::Result::eSuccess);
}

void CVkRenderHandle::destroyShader(FVkShader& shader)
{
	for (auto& pipeline : shader.handles)
		context->destroyPipeline(pipeline);

	for (auto& shader_module : shader.shaderModules)
		context->destroyShaderModule(shader_module);

	context->destroyPipelineLayout(shader.layout);

	for (auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		context->destroyDescriptorSetLayout(descriptor_set_layout);
}
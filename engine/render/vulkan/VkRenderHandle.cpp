#include "VkRenderHandle.h"

#include <VkShaderReflection.h>

#include <logger/logger.h>

using namespace engine::render::vulkan;

const std::vector<uint32_t> vertex_shader{ 119734787, 65536, 851979, 40, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134, 0, 196622, 0, 1, 524303, 0, 4, 1852399981, 0, 9, 12, 26, 196611, 2, 460, 655364, 1197427783, 1279741775, 1885560645, 1953718128, 1600482425, 1701734764, 1919509599, 1769235301, 25974, 524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701, 1952671090, 6649449, 262149, 4, 1852399981, 0, 262149, 9, 1433695599, 86, 393221, 12, 1449094247, 1702130277, 1684949368, 30821, 393221, 24, 1348430951, 1700164197, 2019914866, 0, 393222, 24, 0, 1348430951, 1953067887, 7237481, 196613, 26, 0, 262215, 9, 30, 0, 262215, 12, 11, 42, 327752, 24, 0, 11, 0, 196679, 24, 2, 131091, 2, 196641, 3, 2, 196630, 6, 32, 262167, 7, 6, 2, 262176, 8, 3, 7, 262203, 8, 9, 3, 262165, 10, 32, 1, 262176, 11, 1, 10, 262203, 11, 12, 1, 262187, 10, 14, 1, 262187, 10, 16, 2, 262167, 23, 6, 4, 196638, 24, 23, 262176, 25, 3, 24, 262203, 25, 26, 3, 262187, 10, 27, 0, 262187, 6, 29, 1073741824, 262187, 6, 31, 1065353216, 262187, 6, 34, 0, 262176, 38, 3, 23, 327734, 2, 4, 0, 3, 131320, 5, 262205, 10, 13, 12, 327876, 10, 15, 13, 14, 327879, 10, 17, 15, 16, 262255, 6, 18, 17, 262205, 10, 19, 12, 327879, 10, 20, 19, 16, 262255, 6, 21, 20, 327760, 7, 22, 18, 21, 196670, 9, 22, 262205, 7, 28, 9, 327822, 7, 30, 28, 29, 327760, 7, 32, 31, 31, 327811, 7, 33, 30, 32, 327761, 6, 35, 33, 0, 327761, 6, 36, 33, 1, 458832, 23, 37, 35, 36, 34, 31, 327745, 38, 39, 26, 27, 196670, 39, 37, 65789, 65592 };
const std::vector<uint32_t> fragment_shader{ 119734787, 65536, 851979, 19, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134, 0, 196622, 0, 1, 458767, 4, 4, 1852399981, 0, 9, 12, 196624, 4, 7, 196611, 2, 460, 655364, 1197427783, 1279741775, 1885560645, 1953718128, 1600482425, 1701734764, 1919509599, 1769235301, 25974, 524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701, 1952671090, 6649449, 262149, 4, 1852399981, 0, 327685, 9, 1131705711, 1919904879, 0, 262149, 12, 1448439401, 0, 262215, 9, 30, 0, 262215, 12, 30, 0, 131091, 2, 196641, 3, 2, 196630, 6, 32, 262167, 7, 6, 4, 262176, 8, 3, 7, 262203, 8, 9, 3, 262167, 10, 6, 2, 262176, 11, 1, 10, 262203, 11, 12, 1, 262187, 6, 14, 0, 262187, 6, 15, 1065353216, 327734, 2, 4, 0, 3, 131320, 5, 262205, 10, 13, 12, 327761, 6, 16, 13, 0, 327761, 6, 17, 13, 1, 458832, 7, 18, 16, 17, 14, 15, 196670, 9, 18, 65789, 65592 };

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

vk::PipelineStageFlags2KHR get_translation_stage_flags(vk::ImageLayout layout)
{
	switch (layout)
	{
	case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits2KHR::eTopOfPipe;
	case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits2KHR::eHost;
	case vk::ImageLayout::eTransferDstOptimal:
	case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits2KHR::eTransfer;
	case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput;
	case vk::ImageLayout::eDepthAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
	case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::PipelineStageFlagBits2KHR::eFragmentShadingRateAttachmentKHR;
	case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits2KHR::eFragmentShader | vk::PipelineStageFlagBits2KHR::eVertexShader;
	case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits2KHR::eBottomOfPipe;
	default: return vk::PipelineStageFlagBits2KHR::eNone;
	}
}

vk::AccessFlags2KHR get_translation_access_flags(vk::ImageLayout layout)
{
	switch (layout)
	{
	case vk::ImageLayout::eUndefined:
	case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits2KHR::eNone;
	case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits2KHR::eHostWrite;
	case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits2KHR::eColorAttachmentRead | vk::AccessFlagBits2KHR::eColorAttachmentWrite;
	case vk::ImageLayout::eDepthAttachmentOptimal: return vk::AccessFlagBits2KHR::eDepthStencilAttachmentRead | vk::AccessFlagBits2KHR::eDepthStencilAttachmentWrite;
	case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::AccessFlagBits2KHR::eFragmentShadingRateAttachmentReadKHR;
	case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eInputAttachmentRead;
	case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits2KHR::eTransferRead;
	case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits2KHR::eTransferWrite;
	default: return vk::AccessFlagBits2KHR::eNone;
	}
}

vk::ImageLayout get_render_target_final_layout(const FVkTexture& texture)
{
	if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eColorAttachment)
	{
		if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eTransferSrc)
			return vk::ImageLayout::eTransferSrcOptimal;
		if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eSampled && texture.initialData.usageFlags & vk::ImageUsageFlagBits::eStorage)
			return vk::ImageLayout::eGeneral;
		else if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eSampled)
			return vk::ImageLayout::eShaderReadOnlyOptimal;
		else if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eInputAttachment)
		{
			if(texture.initialData.usageFlags & vk::ImageUsageFlagBits::eSampled)
				return vk::ImageLayout::eShaderReadOnlyOptimal;
			else
				return vk::ImageLayout::eColorAttachmentOptimal;
		}
	}
	else if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eDepthStencilAttachment)
	{
		if (texture.initialData.usageFlags & vk::ImageUsageFlagBits::eSampled)
			return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
		else
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	return vk::ImageLayout::ePresentSrcKHR;
}

FVkImageTranslationInfo get_texture_layout_translation_info(const FVkTexture& texture, const FVkTextureTranslationInfo& translation_info)
{
	FVkImageTranslationInfo image_layout_translation{};
	image_layout_translation.srcStageMask = get_translation_stage_flags(translation_info.oldLayout);
	image_layout_translation.srcAccessMask = get_translation_access_flags(translation_info.oldLayout);
	image_layout_translation.oldLayout = translation_info.oldLayout;
	image_layout_translation.dstStageMask = get_translation_stage_flags(translation_info.newLayout);
	image_layout_translation.dstAccessMask = get_translation_access_flags(translation_info.newLayout);
	image_layout_translation.newLayout = translation_info.newLayout;
	image_layout_translation.handle = texture.image.handle;

	image_layout_translation.subresourceRange.aspectMask = texture.initialData.aspectFlags;
	image_layout_translation.subresourceRange.baseArrayLayer = translation_info.baseArrayLayer;
	image_layout_translation.subresourceRange.layerCount = texture.totalLayers;
	image_layout_translation.subresourceRange.baseMipLevel = translation_info.baseMipLevel;
	image_layout_translation.subresourceRange.levelCount = texture.initialData.mipLevelCount;
	
	return image_layout_translation;
}

bool is_depth_only_format(vk::Format format)
{
	return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
}

CVkRenderHandle::~CVkRenderHandle()
{

}

void CVkRenderHandle::create(const FRenderContextCreateInfo& context_ci)
{
	context = std::make_unique<CVkRenderContext>();
	context->create(context_ci);

	resources = std::make_unique<CVkResourceContext>();
	resources->registerResource<FVkTexture>();
	resources->registerResource<FVkShader>();

	auto& swapchain = context->getSwapchain();

	for (uint32_t idx = 0u; idx < swapchain.framesInFlight; ++idx)
	{
		FVkTexture texture{};
		texture.isRenderTarget = true;
		texture.initialData.format = swapchain.format;
		texture.image.handle = swapchain.images.at(idx);
		texture.imageViews.emplace_back(swapchain.imageViews.at(idx));
		texture.initialData.aspectFlags = vk::ImageAspectFlagBits::eColor;

		swapchainTargets.emplace_back(resources->add<FVkTexture>(std::format("present{}", idx), std::move(texture)));
	}

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = swapchain.framesInFlight;
	command_buffer_ci.mask = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "Graphics";
#endif

	auto result = context->createCommandBuffer(command_buffer_ci, commandBuffer);

	FVkGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = vk::CullModeFlagBits::eBack;
	shader_ci.frontFace = vk::FrontFace::eCounterClockwise;
	shader_ci.primitiveTopology = vk::PrimitiveTopology::eTriangleList;
	shader_ci.colorAttachmentFormats.emplace_back(swapchain.format);
	shader_ci.shaderStageCode[vk::ShaderStageFlagBits::eVertex] = vertex_shader;
	shader_ci.shaderStageCode[vk::ShaderStageFlagBits::eFragment] = fragment_shader;
#ifndef BUILD_WITHOUT_DEBUGGIND
	shader_ci.name = "test_shader_uv";
#endif

	createShader(shader_ci, testShader);
}

void CVkRenderHandle::updateSurface(int32_t surface_width, int32_t surface_height)
{
	if (this->surface_width != surface_width || this->surface_height != surface_height)
	{
		this->surface_width = surface_width;
		this->surface_height = surface_height;

		auto& logical_device = context->getLogicalDevice();
		logical_device.waitIdle();

		context->cleanupSwapchain();
		context->createSwapchain(surface_width, surface_height);
		context->swapchain.currentFrame = 0u;

		auto& swapchain = context->getSwapchain();
		auto& textures = resources->data<FVkTexture>();
		
		// Update swapchain render targets first
		size_t swapchainFrameIndex{ 0ull };
		for (const auto& swapchain_target : swapchainTargets)
		{
			auto& target = textures.at(swapchain_target);
			target.imageViews.clear();

			target.image.handle = swapchain.images.at(swapchainFrameIndex);
			target.imageViews.emplace_back(swapchain.imageViews.at(swapchainFrameIndex));

			swapchainFrameIndex++;
		}

		// Updating render targets
		
		for (size_t texture_id = 0ull; textures.size(); ++texture_id)
		{
			auto is_swapchain_image = std::find(swapchainTargets.begin(), swapchainTargets.end(), texture_id);
			if (is_swapchain_image != swapchainTargets.end())
				continue;

			auto& target = textures.at(texture_id);
			if (!target.isRenderTarget)
				continue;

			for (auto& image_view : target.imageViews)
				context->destroyImageView(image_view);
			target.imageViews.clear();

			context->destroyImage(target.image);

			FVkTextureCreateInfo texture_ci{};
			texture_ci.extent = target.initialData.extent;
			texture_ci.extent.width = swapchain.extent.width;
			texture_ci.extent.height = swapchain.extent.height;
			texture_ci.aspectFlags = target.initialData.aspectFlags;
			texture_ci.usageFlags = target.initialData.usageFlags;
			texture_ci.format = target.initialData.format;
			texture_ci.layout = target.initialData.layout;
			texture_ci.mipLevelCount = target.initialData.mipLevelCount;
			texture_ci.arrayLayersCount = target.initialData.arrayLayersCount;
			texture_ci.numDimensions = target.initialData.numDimensions;
			texture_ci.createSampler = false;

			createTexture(texture_ci, target);
		}

		// Update all render targets here
		for (const auto& texture_id : swapchainTargets)
		{

		}
	}
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

	draw();
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
	texture.initialData.format = texture_ci.format;
	texture.initialData.usageFlags = texture_ci.usageFlags;
	texture.initialData.aspectFlags = texture_ci.aspectFlags;
	texture.initialData.extent = texture_ci.extent;
	texture.initialData.layout = texture_ci.layout;
	texture.initialData.imageTypeFlags = texture_ci.imageTypeFlags;
	texture.initialData.mipLevelCount = texture_ci.mipLevelCount;
	texture.initialData.arrayLayersCount = texture_ci.arrayLayersCount;
	texture.initialData.numDimensions = texture_ci.numDimensions;
	texture.initialData.sampleCount = texture_ci.sampleCount;
	
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

	if ((texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT) && (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT))
		image_ci.ci.arrayLayers = texture_ci.arrayLayersCount * 6u;
	else if (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT)
		image_ci.ci.arrayLayers = texture_ci.arrayLayersCount;
	else if (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT)
		image_ci.ci.arrayLayers = 6u;
	else
		image_ci.ci.arrayLayers = 1u;

	texture.totalLayers = image_ci.ci.arrayLayers;

	image_ci.memoryUsage = vma::MemoryUsage::eUnknown;
	image_ci.ci.format = texture_ci.format;
	image_ci.ci.initialLayout = texture_ci.layout;
	image_ci.ci.usage = texture_ci.usageFlags;
	image_ci.ci.samples = texture_ci.sampleCount;
	image_ci.ci.sharingMode = vk::SharingMode::eExclusive;
	image_ci.ci.flags = (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(context->createImage(image_ci, texture.image));

	FVkImageViewCreateInfo image_view_ci{};
	
	if ((texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT) && (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT))
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	else if (texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if(texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT)
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	else if(texture_ci.imageTypeFlags & TEXTURE_IMAGE_TYPE_3D_FLAG_BIT)
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

	FRenderApiErrorInfo res = make_api_error(vk::ObjectType::eSampler, vk::Result::eSuccess);
	if(texture_ci.createSampler)
		res = context->createSampler(sampler_ci, texture.sampler);

	return res;
}

void CVkRenderHandle::destroyTexture(FVkTexture& texture)
{
	context->destroySampler(texture.sampler);
	
	for (auto& image_view : texture.imageViews)
		context->destroyImageView(image_view);
	texture.imageViews.clear();

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
	pipeline_ci.colorAttachmentFormats = shader_ci.colorAttachmentFormats;
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

void CVkRenderHandle::beginRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info)
{
	auto& textures = resources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};

	// Prepare render targets
	std::vector<vk::RenderingAttachmentInfoKHR> color_attachments{};
	for (const auto& target_id : rendering_info.colorAttachments)
	{
		vk::ClearValue clear_color{ {0.0f, 0.0f, 0.0f, 0.0f} };

		auto& render_target = textures.at(target_id);
		
		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		texture_translation_info.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		color_attachments.emplace_back();
		auto& attachment_info = color_attachments.back();

		// TODO: here i binding first image view to rendering, 
		// but it is incorrect, because other image views can be 
		// mip levels, it means that current code can't draw in mip 
		// levels or array layers.
		attachment_info.imageView = render_target.imageViews.front(); 
		attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
		attachment_info.loadOp = vk::AttachmentLoadOp::eClear;
		attachment_info.storeOp = vk::AttachmentStoreOp::eStore;
		attachment_info.clearValue = clear_color;
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if(rendering_info.depthAttachment != invalid_handle)
	{
		vk::ClearValue clear_color{ {0.f, 0u} };

		auto& depth_target = textures.at(rendering_info.depthAttachment);

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		texture_translation_info.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(depth_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		depth_attachment.imageView = depth_target.imageViews.front();
		depth_attachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
		depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depth_attachment.clearValue = clear_color;
	}

	// Translate all render target layouts
	processMemoryBarriers(command_buffer, translation_info);

	vk::RenderingInfoKHR begin_rendering_info{};
	begin_rendering_info.renderArea = rendering_info.renderArea;
	begin_rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
	begin_rendering_info.pColorAttachments = color_attachments.data();
	begin_rendering_info.layerCount = 1u;

	if (rendering_info.depthAttachment != invalid_handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures.at(rendering_info.depthAttachment);
		if (!is_depth_only_format(depth_target.initialData.format))
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	// Set viewport
	vk::Viewport viewport{};
	if (rendering_info.flipViewport)
	{
		viewport.width = static_cast<float>(rendering_info.renderArea.extent.width);
		viewport.height = -static_cast<float>(rendering_info.renderArea.extent.height);
		viewport.x = 0.f;
		viewport.y = static_cast<float>(rendering_info.renderArea.extent.height);
	}
	else
	{
		viewport.width = static_cast<float>(rendering_info.renderArea.extent.width);
		viewport.height = static_cast<float>(rendering_info.renderArea.extent.height);
		viewport.x = 0.f;
		viewport.y = 0.f;
	}

	auto& current_buffer = command_buffer.handles.at(commandBuffer.commandBufferIndex);
	current_buffer.beginRenderingKHR(begin_rendering_info);

	current_buffer.setViewport(0, 1, &viewport);
	current_buffer.setScissor(0, 1, &rendering_info.renderArea);
}

void CVkRenderHandle::endRendering(const FVkCommandBuffer command_buffer, const FVkRenderingInfo& rendering_info)
{
	auto& textures = resources->data<FVkTexture>();

	auto& current_buffer = command_buffer.handles.at(commandBuffer.commandBufferIndex);
	current_buffer.endRenderingKHR();

	FVkTranslationInfo translation_info{};

	for (const auto& target_id : rendering_info.colorAttachments)
	{
		auto& render_target = textures.at(target_id);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		texture_translation_info.newLayout = get_render_target_final_layout(render_target);
		auto target_layout_translation = get_texture_layout_translation_info(render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	if (rendering_info.depthAttachment != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment);

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		texture_translation_info.newLayout = get_render_target_final_layout(depth_target);
		auto target_layout_translation = get_texture_layout_translation_info(depth_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}
	
	// Translate all render target layouts
	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderHandle::processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info)
{
	std::vector<vk::ImageMemoryBarrier2KHR> image_barriers{};
	for (const auto& image_info : translation_info.imageTranslationInfos)
	{
		image_barriers.emplace_back();
		auto& image_barrier = image_barriers.back();

		// Translate from
		image_barrier.srcStageMask = image_info.srcStageMask;
		image_barrier.srcAccessMask = image_info.srcAccessMask;
		image_barrier.oldLayout = image_info.oldLayout;
		image_barrier.srcQueueFamilyIndex = image_info.srcQueueFamilyIndex;

		// Translate to
		image_barrier.dstStageMask = image_info.dstStageMask;
		image_barrier.dstAccessMask = image_info.dstAccessMask;
		image_barrier.newLayout = image_info.newLayout;
		image_barrier.dstQueueFamilyIndex = image_info.dstQueueFamilyIndex;

		image_barrier.image = image_info.handle;
		image_barrier.subresourceRange = image_info.subresourceRange;
	}

	std::vector<vk::BufferMemoryBarrier2KHR> buffer_barriers{};
	for (const auto& buffer_info : translation_info.bufferTranslationInfos)
	{
		buffer_barriers.emplace_back();
		auto& buffer_barrier = buffer_barriers.back();

		buffer_barrier.srcStageMask = buffer_info.srcStageMask;
		buffer_barrier.srcAccessMask = buffer_info.srcAccessMask;
		buffer_barrier.srcQueueFamilyIndex = buffer_info.srcQueueFamilyIndex;

		buffer_barrier.dstStageMask = buffer_info.dstStageMask;
		buffer_barrier.dstAccessMask = buffer_info.dstAccessMask;
		buffer_barrier.dstQueueFamilyIndex = buffer_info.dstQueueFamilyIndex;

		buffer_barrier.buffer = buffer_info.handle;
		buffer_barrier.size = buffer_info.size;
		buffer_barrier.offset = buffer_info.offset;
	}

	std::vector<vk::MemoryBarrier2KHR> memory_barriers{};
	for (const auto& memory_info : translation_info.memoryTranslationInfos)
	{
		memory_barriers.emplace_back();
		auto& memory_barrier = memory_barriers.back();

		memory_barrier.srcStageMask = memory_info.srcStageMask;
		memory_barrier.srcAccessMask = memory_info.srcAccessMask;

		memory_barrier.dstStageMask = memory_info.dstStageMask;
		memory_barrier.dstAccessMask = memory_info.dstAccessMask;
	}

	vk::DependencyInfoKHR dependency_info{};
	dependency_info.imageMemoryBarrierCount = static_cast<uint32_t>(image_barriers.size());
	dependency_info.pImageMemoryBarriers = image_barriers.data();
	dependency_info.bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_barriers.size());
	dependency_info.pBufferMemoryBarriers = buffer_barriers.data();
	dependency_info.memoryBarrierCount = static_cast<uint32_t>(memory_barriers.size());
	dependency_info.pMemoryBarriers = memory_barriers.data();
	dependency_info.dependencyFlags = vk::DependencyFlagBits::eByRegion;

	auto& current_buffer = command_buffer.handles.at(command_buffer.commandBufferIndex);
	current_buffer.pipelineBarrier2KHR(dependency_info);
}

void CVkRenderHandle::processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info)
{
	FVkTranslationInfo translation_info{};
	translation_info.imageTranslationInfos.emplace_back(image_translation_info);
	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderHandle::textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info)
{
	auto image_layout_translation = get_texture_layout_translation_info(texture, translation_info);
	processImageLayoutTranslation(command_buffer, image_layout_translation);
}

void CVkRenderHandle::draw()
{
	auto& swapchain = context->getSwapchain();

	FVkRenderingInfo rendering_info{};
	rendering_info.flipViewport = true;
	rendering_info.renderArea.extent = swapchain.extent;
	rendering_info.scissorRect = rendering_info.renderArea;
	rendering_info.colorAttachments.emplace_back(swapchainTargets.at(swapchain.currentFrame));

	beginRendering(commandBuffer, rendering_info);

	auto& command_buffer = commandBuffer.handles.at(commandBuffer.commandBufferIndex);
	for (auto& pipeline_handle : testShader.handles)
	{
		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_handle);
		command_buffer.draw(3, 1, 0, 0);
	}

	endRendering(commandBuffer, rendering_info);
}
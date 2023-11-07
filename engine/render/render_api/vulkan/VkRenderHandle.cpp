#include "VkRenderHandle.h"

#include <VkShaderReflection.h>
#include <VkCommon.h>

#include <logger/logger.h>

#include <unordered_set>

using namespace engine;
using namespace engine::render;
using namespace engine::render::vulkan;

const std::vector<uint32_t> vertex_shader{ 119734787, 65536, 851979, 40, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134, 0, 196622, 0, 1, 524303, 0, 4, 1852399981, 0, 9, 12, 26, 196611, 2, 460, 655364, 1197427783, 1279741775, 1885560645, 1953718128, 1600482425, 1701734764, 1919509599, 1769235301, 25974, 524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701, 1952671090, 6649449, 262149, 4, 1852399981, 0, 262149, 9, 1433695599, 86, 393221, 12, 1449094247, 1702130277, 1684949368, 30821, 393221, 24, 1348430951, 1700164197, 2019914866, 0, 393222, 24, 0, 1348430951, 1953067887, 7237481, 196613, 26, 0, 262215, 9, 30, 0, 262215, 12, 11, 42, 327752, 24, 0, 11, 0, 196679, 24, 2, 131091, 2, 196641, 3, 2, 196630, 6, 32, 262167, 7, 6, 2, 262176, 8, 3, 7, 262203, 8, 9, 3, 262165, 10, 32, 1, 262176, 11, 1, 10, 262203, 11, 12, 1, 262187, 10, 14, 1, 262187, 10, 16, 2, 262167, 23, 6, 4, 196638, 24, 23, 262176, 25, 3, 24, 262203, 25, 26, 3, 262187, 10, 27, 0, 262187, 6, 29, 1073741824, 262187, 6, 31, 1065353216, 262187, 6, 34, 0, 262176, 38, 3, 23, 327734, 2, 4, 0, 3, 131320, 5, 262205, 10, 13, 12, 327876, 10, 15, 13, 14, 327879, 10, 17, 15, 16, 262255, 6, 18, 17, 262205, 10, 19, 12, 327879, 10, 20, 19, 16, 262255, 6, 21, 20, 327760, 7, 22, 18, 21, 196670, 9, 22, 262205, 7, 28, 9, 327822, 7, 30, 28, 29, 327760, 7, 32, 31, 31, 327811, 7, 33, 30, 32, 327761, 6, 35, 33, 0, 327761, 6, 36, 33, 1, 458832, 23, 37, 35, 36, 34, 31, 327745, 38, 39, 26, 27, 196670, 39, 37, 65789, 65592 };
const std::vector<uint32_t> fragment_shader{ 119734787, 65536, 851979, 19, 0, 131089, 1, 393227, 1, 1280527431, 1685353262, 808793134, 0, 196622, 0, 1, 458767, 4, 4, 1852399981, 0, 9, 12, 196624, 4, 7, 196611, 2, 460, 655364, 1197427783, 1279741775, 1885560645, 1953718128, 1600482425, 1701734764, 1919509599, 1769235301, 25974, 524292, 1197427783, 1279741775, 1852399429, 1685417059, 1768185701, 1952671090, 6649449, 262149, 4, 1852399981, 0, 327685, 9, 1131705711, 1919904879, 0, 262149, 12, 1448439401, 0, 262215, 9, 30, 0, 262215, 12, 30, 0, 131091, 2, 196641, 3, 2, 196630, 6, 32, 262167, 7, 6, 4, 262176, 8, 3, 7, 262203, 8, 9, 3, 262167, 10, 6, 2, 262176, 11, 1, 10, 262203, 11, 12, 1, 262187, 6, 14, 0, 262187, 6, 15, 1065353216, 327734, 2, 4, 0, 3, 131320, 5, 262205, 10, 13, 12, 327761, 6, 16, 13, 0, 327761, 6, 17, 13, 1, 458832, 7, 18, 16, 17, 14, 15, 196670, 9, 18, 65789, 65592 };

CVkRenderHandle::~CVkRenderHandle()
{
	auto& logical_device = context->getLogicalDevice();
	logical_device.waitIdle();

	for(auto& command_buffer : commandBuffers)
		context->destroyCommandBuffer(command_buffer);

	// Destroy all textures
	auto& textures = resources->data<FVkTexture>();
	for (resource_handle_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
	{
		auto& texture_instances = textures.at(texture_id);
		for (auto& instance : texture_instances)
		{
			if (instance.initialData.base.isSwapchainTarget)
				continue;

			destroyTexture(instance);
		}
	}

	// Destroy all shaders
	auto& shaders = resources->data<FVkShader>();
	for (resource_handle_t shader_id = 0ull; shader_id < shaders.size(); ++shader_id)
	{
		auto& shader_instances = shaders.at(shader_id);
		for(auto& instance : shader_instances)
			destroyShader(instance);
	}
}

void CVkRenderHandle::create(const FRenderContextCreateInfo& context_ci)
{
	context = std::make_unique<CVkRenderContext>();
	context->create(context_ci);

	resources = std::make_unique<CResourceContainer>();
	resources->registerResource<FVkTexture>();
	resources->registerResource<FVkShader>();
	resources->registerResource<FVkShaderReflectionData>();

	auto& swapchain = context->getSwapchain();

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = swapchain.framesInFlight;
	command_buffer_ci.mask = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "Graphics";
#endif

	auto& command_buffer = commandBuffers.emplace_back();
	auto result = context->createCommandBuffer(command_buffer_ci, command_buffer);
}

FResourceView CVkRenderHandle::createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info)
{
	// Create texture handle
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.base = texture_ci;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eRepeat;
	vk_texture_ci.enableAnisotropy = true;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	FVkTexture texture{};
	createTexture(vk_texture_ci, texture);

	auto& upload_data = textureUploadInfos.emplace_back(std::move(upload_info));
	upload_data.textureId = resources->add<FVkTexture>(name.c_str(), std::move(texture));

	return { upload_data.textureId, RESOURCE_TYPE_TEXTURE_BIT };
}

FResourceView CVkRenderHandle::createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.base = texture_ci;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eRepeat;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	FVkTexture texture{};
	texture.isRenderTarget = true;

	createTexture(vk_texture_ci, texture);

	return { resources->add<FVkTexture>(name.c_str(), std::move(texture)), RESOURCE_TYPE_RENDER_TARGET_BIT };
}

FResourceView CVkRenderHandle::createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	auto& swapchain = context->getSwapchain();

	FResourceView resource_view{};

	for (uint32_t idx = 0u; idx < swapchain.framesInFlight; ++idx)
	{
		if (texture_ci.isSwapchainTarget)
		{
			FVkTexture texture{};
			texture.isRenderTarget = true;
			texture.initialData.base = texture_ci;
			texture.image.handle = swapchain.images.at(idx);
			texture.imageViews.emplace_back(swapchain.imageViews.at(idx));

			resource_view.id = resources->add<FVkTexture>(name.c_str(), std::move(texture));
			resource_view.mask = RESOURCE_TYPE_RENDER_TARGET_BIT;
		}
		else
			resource_view = createRenderTarget(name, texture_ci);
	}

	return resource_view;
}

FResourceView CVkRenderHandle::createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	FVkShader shader{};
	CVkShaderReflectionBuilder reflection_builder{};

	FVkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.primitiveTopology = get_primitive_topology(shader_ci.primitiveTopology);
	pipeline_ci.cullMode = get_cull_mode_flags(shader_ci.cullMode);
	pipeline_ci.frontFace = get_front_face(shader_ci.frontFace);

	// Get color attachment formats
	auto& textures = resources->data<FVkTexture>();
	for (auto& attachment : shader_ci.colorAttachments)
	{
		auto& texture = textures.at(attachment.id).at(0);
		pipeline_ci.colorAttachmentFormats.emplace_back(static_cast<vk::Format>(texture.initialData.base.format));
	}

	// Get depth format
	if(shader_ci.depthAttachment.id != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.depthAttachment.id).at(0);
		pipeline_ci.depthAttachmentFormat = static_cast<vk::Format>(texture.initialData.base.format);
	}

	// Get stencil format
	if (shader_ci.stencilAttachment.id != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.stencilAttachment.id).at(0);
		pipeline_ci.stencilAttachmentFormat = static_cast<vk::Format>(texture.initialData.base.format);
	}

	pipeline_ci.sampleCount = vk::SampleCountFlagBits::e1;
	pipeline_ci.patchControlPoints = 0;

	for (auto& vertex_binding : shader_ci.vertexDeclaration.bindingDescriptions)
	{
		auto& description = pipeline_ci.vertexBindingDescriptions.emplace_back();
		description.binding = vertex_binding.binding;
		description.stride = vertex_binding.stride;
		description.inputRate = vk::VertexInputRate::eVertex;
	}

	for (auto& vertex_binding : shader_ci.vertexDeclaration.attributeDescriptions)
	{
		auto& description = pipeline_ci.vertexAttributeDescriptions.emplace_back();
		description.location = vertex_binding.location;
		description.binding = vertex_binding.binding;
		description.format = static_cast<vk::Format>(getApiFormat(vertex_binding.format));
		description.offset = vertex_binding.offset;
	}

	// Create shader modules and collect reflection data
	uint32_t shader_index{ 0u };
	for (const auto& [stage, code] : shader_stages)
	{
		auto shader_stage = get_shader_stage(stage);
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", name, vk::to_string(shader_stage), shader_index);
#endif

		auto& shader_module = shader.shaderModules.emplace_back();
		context->createShaderModule(shader_module_ci, shader_module);

		auto& stage_ci = pipeline_ci.shaderStages.emplace_back();
		stage_ci.stage = shader_stage;
		stage_ci.module = shader_module;
		stage_ci.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(shader_stage, *code);
		shader_index++;
	}

	auto reflection = reflection_builder.build();

	// Update shader stage specialization info
	for (auto& shader_stage : pipeline_ci.shaderStages)
	{
		auto found = reflection.specializationConstantStages.find(shader_stage.stage);
		if (found != reflection.specializationConstantStages.end())
			shader_stage.pSpecializationInfo = &found->second.info;
	}

	shader.reflection = resources->add<FVkShaderReflectionData>("", std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	createShaderResources(resources_info, shader);

	pipeline_ci.layout = shader.layout;

	shader.bindPoint = vk::PipelineBindPoint::eGraphics;

	auto& pipeline = shader.handles.emplace_back();
	context->createPipeline(pipeline_ci, pipeline);

	return { resources->add<FVkShader>(name.c_str(), std::move(shader)), RESOURCE_TYPE_SHADER_BIT };
}

FResourceView CVkRenderHandle::createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	FVkShader shader{};
	CVkShaderReflectionBuilder reflection_builder{};
	std::vector<vk::PipelineShaderStageCreateInfo> pipeline_stages{};

	// Create shader modules and collect reflection data
	for (size_t idx = 0ull; idx < shader_stages.size(); ++idx)
	{
		auto& code = shader_stages.at(idx);
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code.code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", name, vk::to_string(vk::ShaderStageFlagBits::eCompute), idx);
#endif

		auto& shader_module = shader.shaderModules.emplace_back();
		context->createShaderModule(shader_module_ci, shader_module);

		auto& stage = pipeline_stages.emplace_back();
		stage.stage = vk::ShaderStageFlagBits::eCompute;
		stage.module = shader_module;
		stage.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(vk::ShaderStageFlagBits::eCompute, *code.code);
	}

	auto reflection = reflection_builder.build();

	// Update shader stage specialization info
	for (auto& shader_stage : pipeline_stages)
	{
		auto found = reflection.specializationConstantStages.find(shader_stage.stage);
		if (found != reflection.specializationConstantStages.end())
			shader_stage.pSpecializationInfo = &found->second.info;
	}

	shader.reflection = resources->add<FVkShaderReflectionData>("", std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	createShaderResources(resources_info, shader);

	FVkComputePipelineCreateInfo pipeline_ci{};
	pipeline_ci.layout = shader.layout;

	for (size_t idx = 0ull; idx < pipeline_stages.size(); ++idx)
	{
		auto& stage = pipeline_stages.at(idx);
		pipeline_ci.shaderStage = stage;
#ifndef BUILD_WITHOUT_DEBUGGIND
		pipeline_ci.name = std::format("{} #{}", name, idx);
#endif

		auto& pipeline = shader.handles.emplace_back();
		context->createPipeline(pipeline_ci, pipeline);
	}

	shader.bindPoint = vk::PipelineBindPoint::eCompute;

	return { resources->add<FVkShader>(name.c_str(), std::move(shader)), RESOURCE_TYPE_SHADER_BIT };
}

FShaderReflectionData CVkRenderHandle::getShaderReflection(const FResourceView& shader_view)
{
	auto& shader = resources->get<FVkShader>(shader_view.id);
	auto& shader_reflection = resources->get<FVkShaderReflectionData>(shader.reflection);
	FShaderReflectionData reflection_data{};

	for (auto& [id, vkuniform] : shader_reflection.uniforms)
	{
		auto& uniform = reflection_data.uniforms[id];
		uniform.binding = vkuniform.binding;
		uniform.set = vkuniform.set;
		uniform.offset = vkuniform.offset;
		uniform.size = vkuniform.size;
	}

	for (auto& [aid, vkuniform_block] : shader_reflection.uniformBlocks)
	{
		auto& uniform_block = reflection_data.uniformBlocks[aid];
		uniform_block.binding = vkuniform_block.binding;
		uniform_block.set = vkuniform_block.set;
		uniform_block.size = vkuniform_block.size;

		for (auto& [bid, vkuniform] : vkuniform_block.uniforms)
		{
			auto& uniform = uniform_block.uniforms[bid];
			uniform.binding = vkuniform.binding;
			uniform.set = vkuniform.set;
			uniform.offset = vkuniform.offset;
			uniform.size = vkuniform.size;
		}
	}

	for (auto& [aid, vkpush_block] : shader_reflection.pushBlocks)
	{
		auto& push_block = reflection_data.pushBlocks[aid];
		push_block.size = vkpush_block.size;

		for (auto& [bid, vkuniform] : vkpush_block.uniforms)
		{
			auto& uniform = vkpush_block.uniforms[bid];
			uniform.binding = vkuniform.binding;
			uniform.set = vkuniform.set;
			uniform.offset = vkuniform.offset;
			uniform.size = vkuniform.size;
		}
	}

	return reflection_data;
}

FResourceView CVkRenderHandle::getResource(const char* name, uint32_t resource_type)
{
	enum EResourceTypeBits : uint32_t
	{
		RESOURCE_TYPE_TEXTURE_BIT = 1 << 0,
		RESOURCE_TYPE_RENDER_TARGET_BIT = 1 << 1,
		RESOURCE_TYPE_BUFFER_BIT = 1 << 2,
		RESOURCE_TYPE_SHADER_BIT = 1 << 3
	};

	if ((resource_type & RESOURCE_TYPE_TEXTURE_BIT) || (resource_type & RESOURCE_TYPE_RENDER_TARGET_BIT))
		return { resources->getId<FVkTexture>(name), RESOURCE_TYPE_TEXTURE_BIT | RESOURCE_TYPE_RENDER_TARGET_BIT };

	return {};
}

FPoint2D<uint32_t> CVkRenderHandle::getRenderResolution()
{
	auto& swapchain = context->getSwapchain();

	FPoint2D<uint32_t> resolution{};
	resolution.x = swapchain.extent.width;
	resolution.y = swapchain.extent.height;

	return resolution;
}

uint32_t CVkRenderHandle::getDepthFormat()
{
	auto found_format = context->findSupportedFormat
	(
		{
			vk::Format::eD32Sfloat,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint
		},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);

	return static_cast<uint32_t>(found_format);
}

uint32_t CVkRenderHandle::getApiFormat(uint32_t global_format)
{
	switch (global_format)
	{
	case FORMAT_RG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT; break;
	case FORMAT_RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB; break;
	case FORMAT_RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM; break;
	case FORMAT_RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT; break;
	case FORMAT_RGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT; break;
	default: return 0; break;
	}
}

uint32_t CVkRenderHandle::getPresentFormat()
{
	auto& swapchain = context->getSwapchain();
	return static_cast<uint32_t>(swapchain.format);
}

// TODO: update only targets, that have same size with swapchain
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

		// Updating render targets
		
		for (size_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
		{
			size_t swapchainFrameIndex{ 0ull };
			for (auto& target : textures.at(texture_id))
			{
				if (!target.isRenderTarget)
					continue;

				// Update sapchain target
				if (target.initialData.base.isSwapchainTarget)
				{
					target.imageViews.clear();
					target.image.handle = swapchain.images.at(swapchainFrameIndex);
					target.imageViews.emplace_back(swapchain.imageViews.at(swapchainFrameIndex));

					swapchainFrameIndex++;
					continue;
				}

				for (auto& image_view : target.imageViews)
					context->destroyImageView(image_view);
				target.imageViews.clear();

				context->destroyImage(target.image);

				FVkTextureCreateInfo texture_ci{};
				texture_ci.base = target.initialData.base;
				texture_ci.createSampler = false;

				createTexture(texture_ci, target);
			}
		}
	}
}

void CVkRenderHandle::beginFrameCommandBuffers()
{
	uint32_t image_index{ 0u };

	processResourceUpload();

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

	context->beginCommandBuffer(commandBuffers[0], vk::CommandBufferUsageFlagBits::eRenderPassContinue, image_index);

	draw();
}

void CVkRenderHandle::endFrameCommandBuffers()
{
	context->endCommandBuffer(commandBuffers[0]);
	isFrameStarted = false;

	vk::Result presentResult{};
	try { presentResult = context->present(commandBuffers[0]); }
	catch (vk::OutOfDateKHRError err) { presentResult = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("failed to present swap chain image!"); }

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		updateSurface(surface_width, surface_height);

	context->destroyUnusedCommandPools();

	// Frame is finished. We can reuse descriptors
	auto& shaders = resources->data<FVkShader>();
	for (auto& shader_instances : shaders)
	{
		for(auto& shader : shader_instances)
			shader.descriptorUsage.clear();
	}

	// Frame finished. Textures in now again in initial layout
	auto& textures = resources->data<FVkTexture>();
	for (auto& texture_instances : textures)
	{
		for (auto& texture : texture_instances)
		{
			if (texture.isRenderTarget)
				texture.layout = vk::ImageLayout::eUndefined;
		}
	}
}

void CVkRenderHandle::processResourceUpload()
{
	if (textureUploadInfos.empty())
		return;

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1u;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_ci.mask = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "resource_upload";
#endif

	auto& textures = resources->data<FVkTexture>();

	FVkCommandBuffer command_buffer{};
	context->createCommandBuffer(command_buffer_ci, command_buffer);

	context->beginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	// Prepare staging buffers
	std::vector<FVkBuffer> staging_buffers{};
	for (auto& texture_upload : textureUploadInfos)
	{
		FVkBufferCreateInfo staging_buffer_ci{};
		staging_buffer_ci.instanceCount = 1ull;
		staging_buffer_ci.instanceSize = texture_upload.data.size();
		staging_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
		staging_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuOnly;
#ifndef BUILD_WITHOUT_DEBUGGIND
		staging_buffer_ci.name = std::format("texture upload #{}", texture_upload.textureId);
#endif

		auto& staging_buffer = staging_buffers.emplace_back();
		context->createBuffer(staging_buffer_ci, staging_buffer);

		// Copy data to buffer
		void* mapped_buffer_memory{ nullptr };
		context->mapBufferMemory(staging_buffer, &mapped_buffer_memory);
		std::memcpy(mapped_buffer_memory, texture_upload.data.data(), texture_upload.data.size());
		context->unmapBufferMemory(staging_buffer);

		texture_upload.data.clear();
	}

	// Translate all images to transfer dst stage
	std::vector<size_t> generate_mip_textures{};
	FVkTranslationInfo translation_info{};
	for (const auto& texture_upload : textureUploadInfos)
	{
		auto& texture = textures.at(texture_upload.textureId).front();

		auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		texture.layout = texture_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;

		image_translation = get_texture_layout_translation_info(texture, texture_translation_info);

		if (texture.initialData.base.mipLevels > 1u && texture_upload.mipLayerOffsets.empty())
			generate_mip_textures.emplace_back(texture_upload.textureId);
	}

	processMemoryBarriers(command_buffer, translation_info);
	translation_info.imageTranslationInfos.clear();

	// Copy buffer to image
	for (size_t idx = 0ull; idx < textureUploadInfos.size(); ++idx)
	{
		auto& texture_upload = textureUploadInfos.at(idx);
		auto& texture = textures.at(texture_upload.textureId).front();
		auto& staging_buffer = staging_buffers.at(idx);

		auto aspect_mask = get_texture_aspect_flags(texture);

		std::vector<vk::BufferImageCopy2KHR> copy_regions{};
		if (!texture_upload.mipLayerOffsets.empty())
		{
			for (uint32_t layer = 0u; layer < texture.totalLayers; ++layer)
			{
				auto& layer_offsets = texture_upload.mipLayerOffsets.at(layer);
				for (uint32_t mip = 0u; mip < texture.initialData.base.mipLevels; ++mip)
				{
					size_t offset = layer_offsets.at(mip);
					auto& copy_region = copy_regions.emplace_back();
					copy_region.imageSubresource.aspectMask = aspect_mask;
					copy_region.imageSubresource.mipLevel = mip;
					copy_region.imageSubresource.baseArrayLayer = layer;
					copy_region.imageSubresource.layerCount = texture.initialData.base.arrayLayers;
					copy_region.imageExtent.width = texture.initialData.base.width >> mip;
					copy_region.imageExtent.height = texture.initialData.base.height >> mip;
					copy_region.imageExtent.depth = texture.initialData.base.depth;
					copy_region.bufferOffset = offset;
				}
			}
		}
		else
		{
			auto& copy_region = copy_regions.emplace_back();
			copy_region.imageSubresource.aspectMask = aspect_mask;
			copy_region.imageSubresource.mipLevel = 0u;
			copy_region.imageSubresource.baseArrayLayer = 0u;
			copy_region.imageSubresource.layerCount = texture.totalLayers;
			copy_region.imageExtent.width = texture.initialData.base.width;
			copy_region.imageExtent.height = texture.initialData.base.height;
			copy_region.imageExtent.depth = texture.initialData.base.depth;
			copy_region.bufferOffset = 0ull;
		}

		copyBufferToTexture(command_buffer, staging_buffer, texture, copy_regions);
	}

	auto& current_command_buffer = command_buffer.handles.at(command_buffer.commandBufferIndex);
	for (auto& texture_id : generate_mip_textures)
	{
		auto& texture = textures.at(texture_id).front();

		uint32_t mip_width{ texture.initialData.base.width };
		uint32_t mip_heigth{ texture.initialData.base.height };

		for (uint32_t mip = 1u; mip < texture.initialData.base.mipLevels; mip++)
		{
			FVkImageTranslationInfo image_translation_info{};
			image_translation_info.handle = texture.image.handle;
			image_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			image_translation_info.srcStageMask = get_translation_stage_flags(image_translation_info.oldLayout);
			image_translation_info.srcAccessMask = get_translation_access_flags(image_translation_info.oldLayout);
			image_translation_info.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			image_translation_info.dstStageMask = get_translation_stage_flags(image_translation_info.newLayout);
			image_translation_info.dstAccessMask = get_translation_access_flags(image_translation_info.newLayout);
			image_translation_info.subresourceRange.levelCount = mip - 1u;
			image_translation_info.subresourceRange.layerCount = texture.totalLayers;
			processImageLayoutTranslation(command_buffer, image_translation_info);

			auto aspect_mask = get_texture_aspect_flags(texture);

			vk::ImageBlit2KHR image_blit{};
			image_blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			image_blit.srcOffsets[1] = vk::Offset3D{ static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_heigth), 1 };
			image_blit.srcSubresource.aspectMask = aspect_mask;
			image_blit.srcSubresource.mipLevel = image_translation_info.subresourceRange.levelCount;
			image_blit.srcSubresource.baseArrayLayer = 0u;
			image_blit.srcSubresource.layerCount = 1u;
			image_blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			image_blit.dstOffsets[1] = vk::Offset3D{ static_cast<int32_t>(mip_width > 1u ? mip_width / 2u : 1u), static_cast<int32_t>(mip_heigth > 1u ? mip_heigth / 2u : 1u), 1 };
			image_blit.dstSubresource.aspectMask = aspect_mask;
			image_blit.dstSubresource.mipLevel = mip;
			image_blit.dstSubresource.baseArrayLayer = 0u;
			image_blit.dstSubresource.layerCount = 1u;

			vk::BlitImageInfo2KHR blit_info{};
			blit_info.srcImage = texture.image.handle;
			blit_info.srcImageLayout = vk::ImageLayout::eTransferDstOptimal;
			blit_info.dstImage = texture.image.handle;
			blit_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
			blit_info.regionCount = 1u;
			blit_info.pRegions = &image_blit;
			blit_info.filter = vk::Filter::eNearest;

			current_command_buffer.blitImage2KHR(blit_info);

			image_translation_info.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			image_translation_info.srcStageMask = get_translation_stage_flags(image_translation_info.oldLayout);
			image_translation_info.srcAccessMask = get_translation_access_flags(image_translation_info.oldLayout);
			image_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			image_translation_info.dstStageMask = get_translation_stage_flags(image_translation_info.newLayout);
			image_translation_info.dstAccessMask = get_translation_access_flags(image_translation_info.newLayout);
			processImageLayoutTranslation(command_buffer, image_translation_info);

			if (mip_width > 1u)
				mip_width /= 2u;
			if (mip_heigth > 1u)
				mip_heigth /= 2u;
		}
	}

	// Translate images to shader read only layout
	for (const auto& texture_upload : textureUploadInfos)
	{
		auto& texture = textures.at(texture_upload.textureId).front();

		auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		texture.layout = texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		image_translation = get_texture_layout_translation_info(texture, texture_translation_info);
	}

	processMemoryBarriers(command_buffer, translation_info);
	translation_info.imageTranslationInfos.clear();

	// Process buffers and other things

	context->endCommandBuffer(command_buffer);

	context->submitCommandBuffer(command_buffer);

	// TODO: submit command buffer. But with pushing semaphores for synchronize with main command buffer
	context->destroyCommandBuffer(command_buffer);

	// destroy staging buffers
	for (auto& staging_buffer : staging_buffers)
		context->destroyBuffer(staging_buffer);

	textureUploadInfos.clear();
}

FRenderApiErrorInfo CVkRenderHandle::createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture)
{
	texture.initialData.base = texture_ci.base;
	
	FVkImageCreateInfo image_ci{};
	FVkImageViewCreateInfo image_view_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	image_ci.name = texture_ci.name;
#endif

	if (texture_ci.base.depth > 1 || (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_3D_FLAG_BIT))
		image_ci.ci.imageType = vk::ImageType::e3D;
	else
	{
		if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_1D_FLAG_BIT)
			image_ci.ci.imageType = vk::ImageType::e1D;
		else if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_2D_FLAG_BIT)
			image_ci.ci.imageType = vk::ImageType::e2D;
		else if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_3D_FLAG_BIT)
			image_ci.ci.imageType = vk::ImageType::e3D;
		else
			image_ci.ci.imageType = vk::ImageType::e2D;
	}

	image_ci.ci.extent = vk::Extent3D{ texture_ci.base.width, texture_ci.base.height, texture_ci.base.depth };
	image_ci.ci.mipLevels = texture_ci.base.mipLevels;

	image_ci.ci.arrayLayers = texture_ci.base.arrayLayers;
	if ((texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT) && (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT))
	{
		image_ci.ci.arrayLayers *= 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	}
	else if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT)
	{
		image_ci.ci.arrayLayers = 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	}
	else if (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_3D_FLAG_BIT)
		image_view_ci.ci.viewType = vk::ImageViewType::e3D;
	else
	{
		image_ci.ci.arrayLayers = 1u;
		image_view_ci.ci.viewType = vk::ImageViewType::e2D;
	}

	image_view_ci.ci.subresourceRange.layerCount = texture.totalLayers = image_ci.ci.arrayLayers;

	auto usageFlags = get_texture_usage_flags(texture);

	image_ci.memoryUsage = vma::MemoryUsage::eUnknown;
	image_view_ci.ci.format = image_ci.ci.format = static_cast<vk::Format>(texture_ci.base.format);
	image_ci.ci.initialLayout = vk::ImageLayout::eUndefined;
	image_ci.ci.usage = usageFlags;
	texture.initialData.sampleCount = image_ci.ci.samples = texture_ci.sampleCount;
	image_ci.ci.sharingMode = vk::SharingMode::eExclusive;
	image_ci.ci.flags = (texture_ci.base.typeFlags & TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(context->createImage(image_ci, texture.image));

	auto image_aspect = get_texture_aspect_flags(texture);

	image_view_ci.ci.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	image_view_ci.ci.subresourceRange.aspectMask = image_aspect;
	image_view_ci.ci.subresourceRange.baseArrayLayer = 0u;
	image_view_ci.ci.image = texture.image.handle;

	texture.imageViews.resize(texture_ci.base.mipLevels);
	for (uint32_t mip = 0u; mip < texture_ci.base.mipLevels; ++mip)
	{
		image_view_ci.ci.subresourceRange.baseMipLevel = mip;
		image_view_ci.ci.subresourceRange.levelCount = mip == 0u ?texture_ci.base.mipLevels : 1u;
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
	sampler_ci.mipLevels = texture_ci.base.mipLevels;
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

FRenderApiErrorInfo CVkRenderHandle::createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader)
{
	auto& reflection = resources->get<FVkShaderReflectionData>(shader.reflection);

	// Create descriptor set layouts
	for (const auto& [set, layout_bindings] : reflection.descriptorSetLayoutBindings)
	{
		FVkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
		descriptor_set_layout_ci.name = std::format("{}: set #{}", resources_info.name, set);
#endif
		descriptor_set_layout_ci.layoutBindings = layout_bindings; // TODO: may be send pointer to vector here?

		auto& descriptor_set_Layout = shader.descriptorSetLayouts[set];
		RETURN_WHEN_FAULT(context->createDescriptorSetLayout(descriptor_set_layout_ci, descriptor_set_Layout));
	}

	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = resources_info.name;
#endif
	pool_ci.poolSizes = reflection.descriptorPoolSizes;
	pool_ci.usageCount = resources_info.usageCount;
	RETURN_WHEN_FAULT(context->createDescriptorPool(pool_ci, shader.descriptorPool));

	// Create descrioptor sets for each shader usage
	for (size_t idx = 0ull; idx < resources_info.usageCount; ++idx)
	{
		for (const auto& [set, layout] : shader.descriptorSetLayouts)
		{
			FVkDescriptorSetCreateInfo descriptor_set_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
			descriptor_set_ci.name = std::format("{}: set #{} usage #{}", resources_info.name, set, idx);
#endif
			descriptor_set_ci.pool = shader.descriptorPool.handle;
			descriptor_set_ci.layout = layout;

			auto& descriptor_set = shader.descriptorSets[set].emplace_back();
			RETURN_WHEN_FAULT(context->createDescriptorSet(descriptor_set_ci, descriptor_set));
		}
	}

	for (size_t idx = 0ull; idx < resources_info.usageCount; ++idx)
	{
		for (const auto& [id, uniform_block] : reflection.uniformBlocks)
		{
			FVkBufferCreateInfo buffer_ci{};
			buffer_ci.instanceCount = 1u;
			buffer_ci.instanceSize = uniform_block.size;
			buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;

			if (uniform_block.descriptorType == vk::DescriptorType::eUniformBuffer)
				buffer_ci.usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
			else if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
				buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer;

			auto& buffer = shader.uniformBuffers[id].emplace_back();
			RETURN_WHEN_FAULT(context->createBuffer(buffer_ci, buffer));
		}
	}

	// Create pipeline layout
	FVkPipelineLayoutCreateInfo layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	layout_ci.name = resources_info.name;
#endif
	// Send descriptor set layouts to create info
	for (const auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		layout_ci.descriptorSetLayouts.emplace_back(descriptor_set_layout);

	// Prepare push blocks info
	uint32_t current_offset{ 0u };
	for (const auto& [name, block] : reflection.pushBlocks)
	{
		auto& push_constant_range = layout_ci.pushConstantRanges.emplace_back();
		push_constant_range.stageFlags = block.stageFlags;
		push_constant_range.offset = current_offset;
		push_constant_range.size = static_cast<uint32_t>(block.size);
		current_offset += push_constant_range.size;
	}

	return context->createPipelineLayout(layout_ci, shader.layout);
}

// Destroy shader object
void CVkRenderHandle::destroyShader(FVkShader& shader)
{
	for (auto& pipeline : shader.handles)
		context->destroyPipeline(pipeline);

	for (auto& shader_module : shader.shaderModules)
		context->destroyShaderModule(shader_module);

	for (auto& [set, descriptor_sets] : shader.descriptorSets)
	{
		for (auto& descriptor_set : descriptor_sets)
			context->destroyDescriptorSet(shader.descriptorPool, descriptor_set);
	}

	for (auto& [id, buffers] : shader.uniformBuffers)
	{
		for (auto& buffer : buffers)
			context->destroyBuffer(buffer);
	}

	context->destroyDescriptorPool(shader.descriptorPool);

	context->destroyPipelineLayout(shader.layout);

	for (auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		context->destroyDescriptorSetLayout(descriptor_set_layout);
}

void CVkRenderHandle::updateShaderDescriptor(const FVkShaderDescriptorsUpdateInfo& update_info)
{
	auto& shader = resources->get<FVkShader>(update_info.shaderId);
	auto& reflection = resources->get<FVkShaderReflectionData>(shader.reflection);
	auto& swapchain = context->getSwapchain();

	std::unordered_set<uint32_t> updatedDescriptorSets{};
	std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
	std::vector<vk::DescriptorImageInfo> imageDescriptors{};
	std::vector<vk::DescriptorBufferInfo> bufferDescriptors{};

	for (auto& resource_info : update_info.resourceInfos)
	{
		auto& write_descriptor_set = writeDescriptorSets.emplace_back();

		if (resource_info.resourceType == EVkResourceDescriptorType::eTexture)
		{
			auto& texture = resources->get<FVkTexture>(resource_info.resourceId);

			auto found = reflection.uniforms.find(resource_info.resourceReflectionName);
			if (found == reflection.uniforms.end())
			{
				writeDescriptorSets.pop_back();
				continue;
			}

			auto& uniform = found->second;
			updatedDescriptorSets.emplace(uniform.set);

			// TODO: add image view selection support
			auto& image_descriptor = imageDescriptors.emplace_back();
			image_descriptor.imageView = texture.imageViews.front();
			image_descriptor.sampler = texture.sampler;
			image_descriptor.imageLayout = texture.layout;

			auto& current_descriptor_set = shader.descriptorSets[uniform.set].at(shader.descriptorUsage[uniform.set]).handles.at(swapchain.currentFrame);

			write_descriptor_set.descriptorType = uniform.descriptorType;
			write_descriptor_set.dstBinding = uniform.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pImageInfo = &image_descriptor;
			write_descriptor_set.descriptorCount = 1;
		}
		else if (resource_info.resourceType == EVkResourceDescriptorType::eBuffer)
		{
			auto& buffer = resources->get<FVkBuffer>(resource_info.resourceId);

			auto found = reflection.uniformBlocks.find(resource_info.resourceReflectionName);
			if (found == reflection.uniformBlocks.end())
			{
				writeDescriptorSets.pop_back();
				continue;
			}

			auto& uniform = found->second;
			updatedDescriptorSets.emplace(uniform.set);

			auto& buffer_descriptor = bufferDescriptors.emplace_back();
			buffer_descriptor.buffer = buffer.handle;
			buffer_descriptor.range = buffer.size;

			auto& current_descriptor_set = shader.descriptorSets[uniform.set].at(shader.descriptorUsage[uniform.set]).handles.at(swapchain.currentFrame);

			write_descriptor_set.descriptorType = uniform.descriptorType;
			write_descriptor_set.dstBinding = uniform.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pBufferInfo = &buffer_descriptor;
			write_descriptor_set.descriptorCount = 1;
		}
	}

	// This should help me to reuse some descriptors
	for (auto& descriptor_set : updatedDescriptorSets)
		shader.descriptorUsage[descriptor_set]++;

	auto& logical = context->getLogicalDevice();
	logical.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void CVkRenderHandle::bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader)
{
	std::vector<vk::DescriptorSet> bindDescriptorSets{};

	auto& swapchain = context->getSwapchain();
	for (auto& [set, usage_index] : shader.descriptorUsage)
	{
		auto found_descriptor_sets = shader.descriptorSets.find(set);
		if (found_descriptor_sets != shader.descriptorSets.end())
		{
			auto& descriptor_sets = found_descriptor_sets->second;
			auto& target_descriptor_set = descriptor_sets.at(usage_index - 1u);
			bindDescriptorSets.emplace_back(target_descriptor_set.handles.at(swapchain.currentFrame));
		}
	}

	auto& current_command_buffer = command_buffer.handles.at(command_buffer.commandBufferIndex);
	current_command_buffer.bindDescriptorSets(shader.bindPoint, shader.layout, 0, static_cast<uint32_t>(bindDescriptorSets.size()), bindDescriptorSets.data(), 0u, nullptr);
}

void CVkRenderHandle::beginRendering(const FVkCommandBuffer command_buffer, const FBeginRenderingInfo& rendering_info)
{
	auto& textures = resources->data<FVkTexture>();
	auto& swapchain = context->getSwapchain();

	FVkTranslationInfo translation_info{};

	// Prepare render targets
	std::vector<vk::RenderingAttachmentInfoKHR> color_attachments{};
	for (const auto& target_view : rendering_info.colorAttachments)
	{
		vk::ClearValue clear_color{ {0.0f, 0.0f, 0.0f, 0.0f} };

		auto& render_target = textures.at(target_view.id).at(swapchain.currentFrame);
		
		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		render_target.layout = texture_translation_info.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		auto& attachment_info = color_attachments.emplace_back();

		// TODO: here i binding first image view to rendering, 
		// but it is incorrect, because other image views can be 
		// mip levels, it means that current code can't draw in mip 
		// levels or array layers.
		attachment_info.imageView = render_target.imageViews.front(); 
		attachment_info.imageLayout = render_target.layout;
		attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
		attachment_info.loadOp = vk::AttachmentLoadOp::eClear;
		attachment_info.storeOp = vk::AttachmentStoreOp::eStore;
		attachment_info.clearValue = clear_color;
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if(rendering_info.depthAttachment.id != invalid_handle)
	{
		vk::ClearValue clear_color{ {0.f, 0u} };

		auto& depth_target = textures.at(rendering_info.depthAttachment.id).at(swapchain.currentFrame);

		bool is_depth_stencil_attachment = (rendering_info.stencilAttachment.id == rendering_info.depthAttachment.id) && rendering_info.stencilAttachment.id != invalid_handle;

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		depth_target.layout = texture_translation_info.newLayout = is_depth_stencil_attachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(depth_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		depth_attachment.imageView = depth_target.imageViews.front();
		depth_attachment.imageLayout = depth_target.layout;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
		depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depth_attachment.clearValue = clear_color;
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment.id != rendering_info.depthAttachment.id && rendering_info.stencilAttachment.id != invalid_handle)
	{
		vk::ClearValue clear_color{ {0.f, 0u} };

		auto& stencil_target = textures.at(rendering_info.stencilAttachment.id).at(swapchain.currentFrame);

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		stencil_target.layout = texture_translation_info.newLayout = vk::ImageLayout::eStencilAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(stencil_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		stencil_attachment.imageView = stencil_target.imageViews.front();
		stencil_attachment.imageLayout = stencil_target.layout;
		stencil_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		stencil_attachment.loadOp = vk::AttachmentLoadOp::eClear;
		stencil_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		stencil_attachment.clearValue = clear_color;
	}

	// Translate all render target layouts
	processMemoryBarriers(command_buffer, translation_info);

	vk::RenderingInfoKHR begin_rendering_info{};
	begin_rendering_info.renderArea.offset.x = rendering_info.renderArea.offset.x;
	begin_rendering_info.renderArea.offset.y = rendering_info.renderArea.offset.y;
	begin_rendering_info.renderArea.extent.width = rendering_info.renderArea.size.x;
	begin_rendering_info.renderArea.extent.height = rendering_info.renderArea.size.y;
	begin_rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
	begin_rendering_info.pColorAttachments = color_attachments.data();
	begin_rendering_info.layerCount = 1u;

	if (rendering_info.depthAttachment.id != invalid_handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures.at(rendering_info.depthAttachment.id).at(swapchain.currentFrame);
		if (!is_depth_only_format(static_cast<vk::Format>(depth_target.initialData.base.format)) && rendering_info.stencilAttachment.id == rendering_info.depthAttachment.id)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment.id != invalid_handle)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	// Set viewport
	vk::Viewport viewport{};
	if (rendering_info.flipViewport)
	{
		viewport.width = static_cast<float>(rendering_info.renderArea.size.x);
		viewport.height = -static_cast<float>(rendering_info.renderArea.size.y);
		viewport.x = 0.f;
		viewport.y = static_cast<float>(rendering_info.renderArea.size.y);
	}
	else
	{
		viewport.width = static_cast<float>(rendering_info.renderArea.size.x);
		viewport.height = static_cast<float>(rendering_info.renderArea.size.y);
		viewport.x = 0.f;
		viewport.y = 0.f;
	}

	vk::Rect2D scissor_rect{};
	scissor_rect.offset.x = rendering_info.scissorRect.offset.x;
	scissor_rect.offset.y = rendering_info.scissorRect.offset.y;
	scissor_rect.extent.width = rendering_info.scissorRect.size.x;
	scissor_rect.extent.height = rendering_info.scissorRect.size.y;

	auto& current_buffer = command_buffer.handles.at(commandBuffers[0].commandBufferIndex);
	current_buffer.beginRenderingKHR(begin_rendering_info);

	current_buffer.setViewport(0, 1, &viewport);
	current_buffer.setScissor(0, 1, &scissor_rect);
}

void CVkRenderHandle::endRendering(const FVkCommandBuffer command_buffer, const FBeginRenderingInfo& rendering_info)
{
	auto& textures = resources->data<FVkTexture>();
	auto& swapchain = context->getSwapchain();

	auto& current_buffer = command_buffer.handles.at(commandBuffers[0].commandBufferIndex);
	current_buffer.endRenderingKHR();

	FVkTranslationInfo translation_info{};

	for (const auto& target_view : rendering_info.colorAttachments)
	{
		auto& render_target = textures.at(target_view.id).at(swapchain.currentFrame);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		texture_translation_info.newLayout = get_render_target_final_layout(render_target);
		auto target_layout_translation = get_texture_layout_translation_info(render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	if (rendering_info.depthAttachment.id != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment.id).at(swapchain.currentFrame);

		bool is_depth_stencil_attachment = (rendering_info.stencilAttachment.id == rendering_info.depthAttachment.id) && rendering_info.stencilAttachment.id != invalid_handle;

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = is_depth_stencil_attachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimal;
		texture_translation_info.newLayout = get_render_target_final_layout(depth_target);
		auto target_layout_translation = get_texture_layout_translation_info(depth_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	if (rendering_info.stencilAttachment.id != invalid_handle && rendering_info.depthAttachment.id != rendering_info.stencilAttachment.id)
	{
		auto& stencil_target = textures.at(rendering_info.stencilAttachment.id).at(swapchain.currentFrame);;

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eStencilAttachmentOptimal;
		texture_translation_info.newLayout = get_render_target_final_layout(stencil_target);
		auto target_layout_translation = get_texture_layout_translation_info(stencil_target, texture_translation_info);
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
		auto& image_barrier = image_barriers.emplace_back();

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
		auto& buffer_barrier = buffer_barriers.emplace_back();

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
		auto& memory_barrier = memory_barriers.emplace_back();

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

void CVkRenderHandle::copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const std::vector<vk::BufferImageCopy2KHR>& copy_regions)
{
	auto& current_buffer = command_buffer.handles.at(command_buffer.commandBufferIndex);

	vk::CopyBufferToImageInfo2KHR copy_info{};
	copy_info.srcBuffer = buffer.handle;
	copy_info.dstImage = texture.image.handle;
	copy_info.dstImageLayout = texture.layout;
	copy_info.regionCount = static_cast<uint32_t>(copy_regions.size());
	copy_info.pRegions = copy_regions.data();

	current_buffer.copyBufferToImage2KHR(&copy_info);
}

void CVkRenderHandle::draw()
{
	//auto& swapchain = context->getSwapchain();
	//
	//FVkRenderingInfo rendering_info{};
	//rendering_info.flipViewport = true;
	//rendering_info.renderArea.extent = swapchain.extent;
	//rendering_info.scissorRect = rendering_info.renderArea;
	//rendering_info.colorAttachments.emplace_back(swapchainTarget);
	//
	//beginRendering(commandBuffers[0], rendering_info);
	//
	//auto& command_buffer = commandBuffers[0].handles.at(commandBuffers[0].commandBufferIndex);
	//
	//auto& shader = resources->get<FVkShader>(testShader);
	//for (const auto& pipeline_handle : shader.handles)
	//{
	//	command_buffer.bindPipeline(shader.bindPoint, pipeline_handle);
	//	command_buffer.draw(3, 1, 0, 0);
	//}
	//
	//endRendering(commandBuffers[0], rendering_info);
}
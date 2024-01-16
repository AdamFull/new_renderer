#include "VkRenderContext.h"

#include "VkShaderReflection.h"
#include "VkCommon.h"

#include <logger/logger.h>

#include <numeric>
#include <unordered_set>

using namespace engine;
using namespace engine::render;
using namespace engine::render::vulkan;

static const uint32_t k_max_bindless_buffers{ 256u };

static const uint32_t k_max_bindless_resources{ 2048u };
static const size_t k_max_texture_update_per_frame{ 64ull };
static const size_t k_max_buffer_update_per_frame{ 64ull };

template<class _Ty, class ..._Args>
std::string create_object_unique_name(const char* fmt, const _Ty& handle, _Args&&... args)
{
	auto object_handle = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(handle));
	return std::vformat(fmt, std::make_format_args(object_handle, std::forward<_Args>(args)...));
}

CVkRenderContext::~CVkRenderContext()
{
	auto& logical_device = m_pDevice->getLogicalDevice();
	logical_device.waitIdle();

	// Destroy thread contexts
	for (auto& [thread_id, context] : m_perThreadContext)
		destroyThreadContext(context);

	// Destroy all textures
	auto& textures = m_pResources->data<FVkTexture>();
	for (resource_handle_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
	{
		auto& texture_instances = textures.at(texture_id);
		for (auto& instance : texture_instances)
		{
			if (instance->initialData.base.isSwapchainTarget || !instance->isAlive)
				continue;

			destroyTexture(*instance);
		}
	}

	// Destroy all shaders
	auto& shaders = m_pResources->data<FVkShader>();
	for (resource_handle_t shader_id = 0ull; shader_id < shaders.size(); ++shader_id)
	{
		auto& shader_instances = shaders.at(shader_id);
		for (auto& instance : shader_instances)
		{
			if (!instance->isAlive)
				continue;

			destroyShader(*instance);
		}
	}

	// Destroy all buffers
	auto& buffers = m_pResources->data<FVkBuffer>();
	for (resource_handle_t buffer_id = 0ull; buffer_id < buffers.size(); ++buffer_id)
	{
		auto& buffer_instances = buffers.at(buffer_id);
		for (auto& instance : buffer_instances)
		{
			if (!instance->isAlive)
				continue;

			m_pDevice->destroyBuffer(*instance);
		}
	}
}

void CVkRenderContext::create(const FRenderContextCreateInfo& context_ci)
{
	m_pDevice = std::make_unique<CVkRenderDevice>();
	m_pDevice->create(context_ci);

	m_pResources = std::make_unique<CResourceContainer>();
	m_pResources->registerResource<FVkTexture>();
	m_pResources->registerResource<FVkShader>();
	m_pResources->registerResource<FVkBuffer>();
	m_pResources->registerResource<FVkShaderReflectionData>();

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = m_pDevice->m_swapchain.framesInFlight;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "Graphics";
#endif

	createCommandBuffer(command_buffer_ci);

	auto& per_thread = m_perThreadContext[std::this_thread::get_id()];
	auto& per_queue = per_thread.perQueue[EQueueTypeFlagBits::eGraphics];
	m_pDevice->initProfiler(per_queue.commandPool);
}

void CVkRenderContext::createGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci)
{
	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
	pool_ci.poolSizes = { { vk::DescriptorType::eCombinedImageSampler, k_max_bindless_resources }, {vk::DescriptorType::eUniformBuffer, 2u} };
	pool_ci.usageCount = 1u * m_pDevice->m_swapchain.framesInFlight;
	pool_ci.flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = "global:textures";
#endif

	m_pDevice->createDescriptorPool(pool_ci, m_globalDescriptor.descriptorPool);

	// Create descriptor set layout
	FVkDescriptorSetLayoutCreateInfo set_layout_ci{};
	set_layout_ci.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;

	// Frame data
	auto& global_frame_data_binding = set_layout_ci.layoutBindings.emplace_back();
	global_frame_data_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
	global_frame_data_binding.descriptorCount = 1u;
	global_frame_data_binding.binding = 0u;
	global_frame_data_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;

	// Uniform buffer with adresses to another buffers
	auto& global_buffer_references_binding = set_layout_ci.layoutBindings.emplace_back();
	global_buffer_references_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
	global_buffer_references_binding.descriptorCount = 1u;
	global_buffer_references_binding.binding = 1u;
	global_buffer_references_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;

	// Bindles textures
	auto& global_textures_binding = set_layout_ci.layoutBindings.emplace_back();
	global_textures_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	global_textures_binding.descriptorCount = k_max_bindless_resources;
	global_textures_binding.binding = 2u;
	global_textures_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
	global_textures_binding.pImmutableSamplers = nullptr;

	// Descriptor binding extension
	std::vector<vk::DescriptorBindingFlags> bindingFlags{};
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

	vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCIEXT{};
	descriptorSetLayoutBindingFlagsCIEXT.bindingCount = static_cast<uint32_t>(bindingFlags.size());
	descriptorSetLayoutBindingFlagsCIEXT.pBindingFlags = bindingFlags.data();

	set_layout_ci.next = &descriptorSetLayoutBindingFlagsCIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_layout_ci.name = "global:textures";
#endif

	m_pDevice->createDescriptorSetLayout(set_layout_ci, m_globalDescriptor.deecriptorSetLayout);

	// Create descriptor set
	FVkDescriptorSetCreateInfo set_ci{};
	set_ci.descriptorCount = m_pDevice->m_swapchain.framesInFlight;
	set_ci.layout = m_globalDescriptor.deecriptorSetLayout;
	set_ci.pool = m_globalDescriptor.descriptorPool.handle;

	// TODO: Potential problems here in future. I think that max_sets array is incorrect
	std::vector<uint32_t> max_sets(m_pDevice->m_swapchain.framesInFlight, k_max_bindless_resources - 1);
	vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT descriptor_set_variable_descriptor_count_AIEXT{};
	descriptor_set_variable_descriptor_count_AIEXT.descriptorSetCount = m_pDevice->m_swapchain.framesInFlight;
	descriptor_set_variable_descriptor_count_AIEXT.pDescriptorCounts = max_sets.data();
	set_ci.next = &descriptor_set_variable_descriptor_count_AIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_ci.name = "global:textures";
#endif

	m_pDevice->createDescriptorSet(set_ci, m_globalDescriptor.descriptorSet);

	// Create uniform buffers
	auto& physical = m_pDevice->getPhysicalDevice();

	for (auto& [uniform_name, size] : uniform_ci.uniforms)
	{
		FVkBufferCreateInfo buffer_ci{};
		buffer_ci.instanceCount = m_pDevice->m_swapchain.framesInFlight;
		buffer_ci.instanceSize = size;
		buffer_ci.minOffsetAlignment = std::lcm(physical.properties.limits.minUniformBufferOffsetAlignment, physical.properties.limits.nonCoherentAtomSize);
		buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
		buffer_ci.usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
#ifndef BUILD_WITHOUT_DEBUGGIND
		auto* debug_name = debug_mapping::find(uniform_name);

		if(debug_name)
			buffer_ci.name = std::format("global:{}", debug_name);
		else
			buffer_ci.name = std::format("global:{:x}", uniform_name);
#endif

		auto buffer = std::make_unique<FVkBuffer>();
		m_pDevice->createBuffer(buffer_ci, *buffer);

		auto buffer_name = create_object_unique_name("global:{:x}:{:x}", buffer->handle, uniform_name);
		auto buffer_id = m_pResources->add<FVkBuffer>(buffer_name.c_str(), std::move(buffer));
		m_globalDescriptor.uniformBuffers.emplace(uniform_name, buffer_id);
	}
}

void CVkRenderContext::updateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view)
{
	auto& dest_set = m_globalDescriptor.descriptorSet.handles.at(m_pDevice->m_swapchain.currentFrame);

	std::array<vk::DescriptorImageInfo, k_max_texture_update_per_frame> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, k_max_buffer_update_per_frame> buffer_descriptors{};
	std::vector<vk::WriteDescriptorSet> write_descriptor_sets{};

	// Bind and update global uniform
	size_t buffer_count{ 0ull };
	for (const auto& [uniform_name, uniform_data] : data_view.uniforms)
	{
		auto found_buffer = m_globalDescriptor.uniformBuffers.find(uniform_name);
		if (found_buffer == m_globalDescriptor.uniformBuffers.end())
			continue;

		auto& buffer = m_pResources->get<FVkBuffer>(found_buffer->second);
		auto element_size = buffer->size / buffer->instanceCount;

		auto& buffer_descriptor = buffer_descriptors.at(buffer_count);
		buffer_descriptor.buffer = buffer->handle;
		buffer_descriptor.range = element_size;
		buffer_descriptor.offset = m_pDevice->m_swapchain.currentFrame * element_size;

		auto& write_descriptor_set = write_descriptor_sets.emplace_back();
		write_descriptor_set.descriptorType = vk::DescriptorType::eUniformBuffer;
		write_descriptor_set.dstSet = dest_set;
		write_descriptor_set.dstBinding = uniform_data.binding;
		write_descriptor_set.descriptorCount = 1u;
		write_descriptor_set.pBufferInfo = &buffer_descriptor;

		void* mapped_memory{ nullptr };
		m_pDevice->mapBufferMemory(*buffer, &mapped_memory);
		std::memcpy((char*)mapped_memory + buffer_descriptor.offset, uniform_data.data, uniform_data.size);
		m_pDevice->unmapBufferMemory(*buffer);
		buffer_count++;
	}
	
	{
		auto& texture_queue = m_textureToUpdate[m_pDevice->m_swapchain.currentFrame];

		size_t texture_count{ 0ull };
		while (!texture_queue.empty())
		{
			if (texture_count >= k_max_texture_update_per_frame)
				break;

			auto texture_id = texture_queue.front();
			auto& texture = m_pResources->get<FVkTexture>(texture_id);

			auto& image_descriptor = image_descriptors.at(texture_count);
			image_descriptor.imageView = texture->imageViews.front();
			image_descriptor.sampler = texture->sampler;
			image_descriptor.imageLayout = texture->layout;

			auto& write_descriptor_set = write_descriptor_sets.emplace_back();
			write_descriptor_set.dstArrayElement = texture_id;
			write_descriptor_set.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			write_descriptor_set.dstSet = dest_set;
			write_descriptor_set.dstBinding = 2u;
			write_descriptor_set.descriptorCount = 1u;
			write_descriptor_set.pImageInfo = &image_descriptor;

			texture_queue.pop();
			texture_count++;
		}
	}

	// Update descriptor sets if needed
	if (!write_descriptor_sets.empty())
	{
		auto& logical = m_pDevice->getLogicalDevice();
		logical.updateDescriptorSets(static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
	}
}

size_t CVkRenderContext::createTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info)
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

	auto texture = std::make_unique<FVkTexture>();
	createTexture(vk_texture_ci, *texture);

	auto& upload_data = m_textureUploadInfos.emplace_back(std::move(upload_info));
	upload_data.textureId = m_pResources->add<FVkTexture>(name.c_str(), std::move(texture));

	// Add texture to bindless buffer
	{
		for (uint32_t frame = 0u; frame < m_pDevice->m_swapchain.framesInFlight; ++frame)
			m_textureToUpdate[frame].push(upload_data.textureId);
	}

	return upload_data.textureId;
}

size_t CVkRenderContext::getTextureId(const std::string& name)
{
	return m_pResources->getId<FVkTexture>(name.c_str());
}

size_t CVkRenderContext::createRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.base = texture_ci;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eRepeat;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	auto texture = std::make_unique<FVkTexture>();
	texture->isRenderTarget = true;

	createTexture(vk_texture_ci, *texture);

	return m_pResources->add<FVkTexture>(name.c_str(), std::move(texture));
}

size_t CVkRenderContext::createRenderTargets(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	size_t resource_id{ ~0ull };

	for (uint32_t idx = 0u; idx < m_pDevice->m_swapchain.framesInFlight; ++idx)
	{
		if (texture_ci.isSwapchainTarget)
		{
			auto texture = std::make_unique<FVkTexture>();
			texture->isRenderTarget = true;
			texture->initialData.base = texture_ci;
			texture->image.handle = m_pDevice->m_swapchain.images.at(idx);
			texture->imageViews.emplace_back(m_pDevice->m_swapchain.imageViews.at(idx));

			resource_id = m_pResources->add<FVkTexture>(name.c_str(), std::move(texture));
		}
		else
			resource_id = createRenderTarget(name, texture_ci);
	}

	return resource_id;
}

const FTextureCreateInfo& CVkRenderContext::getTextureInfo(size_t texture_id)
{
	auto& texture = m_pResources->get<FVkTexture>(texture_id);
	return texture->initialData.base;
}

void CVkRenderContext::destroyTexture(size_t resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eTexture });
}


size_t CVkRenderContext::createShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	CVkShaderReflectionBuilder reflection_builder{};

	FVkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.primitiveTopology = get_primitive_topology(shader_ci.primitiveTopology);
	pipeline_ci.cullMode = get_cull_mode_flags(shader_ci.cullMode);
	pipeline_ci.frontFace = get_front_face(shader_ci.frontFace);
#ifndef BUILD_WITHOUT_DEBUGGIND
	pipeline_ci.name = name;
#endif

	// Get color attachment formats
	auto& textures = m_pResources->data<FVkTexture>();
	for (auto& attachment : shader_ci.colorAttachments)
	{
		auto& texture = textures.at(attachment).at(0);
		pipeline_ci.colorAttachmentFormats.emplace_back(static_cast<vk::Format>(texture->initialData.base.format));
	}

	// Get depth format
	if(shader_ci.depthAttachment != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.depthAttachment).at(0);
		pipeline_ci.depthAttachmentFormat = static_cast<vk::Format>(texture->initialData.base.format);
	}

	// Get stencil format
	if (shader_ci.stencilAttachment != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.stencilAttachment).at(0);
		pipeline_ci.stencilAttachmentFormat = static_cast<vk::Format>(texture->initialData.base.format);
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
		description.format = static_cast<vk::Format>(vertex_binding.format);
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

		auto& shader_module = shader->shaderModules.emplace_back();
		m_pDevice->createShaderModule(shader_module_ci, shader_module);

		auto& stage_ci = pipeline_ci.shaderStages.emplace_back();
		stage_ci.stage = shader_stage;
		stage_ci.module = shader_module;
		stage_ci.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(shader_stage, *code);
		shader_index++;
	}

	auto reflection = std::make_unique<FVkShaderReflectionData>(reflection_builder.build());

	// Update shader stage specialization info
	for (auto& shader_stage : pipeline_ci.shaderStages)
	{
		auto found = reflection->specializationConstantStages.find(shader_stage.stage);
		if (found != reflection->specializationConstantStages.end())
			shader_stage.pSpecializationInfo = &found->second.info;
	}

	shader->reflection = m_pResources->add<FVkShaderReflectionData>(name.c_str(), std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	auto res = createShaderResources(resources_info, *shader);

	pipeline_ci.layout = shader->layout;

	shader->bindPoint = vk::PipelineBindPoint::eGraphics;

	auto& pipeline = shader->handles.emplace_back();
	m_pDevice->createPipeline(pipeline_ci, pipeline);
	
	shader->isAlive = true;

	auto shader_unique_name = create_object_unique_name("shader:{:x}:{}", shader->handles.back(), vk::to_string(shader->bindPoint));
	return m_pResources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader));
}

size_t CVkRenderContext::createShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  std::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
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

		auto& shader_module = shader->shaderModules.emplace_back();
		m_pDevice->createShaderModule(shader_module_ci, shader_module);

		auto& stage = pipeline_stages.emplace_back();
		stage.stage = vk::ShaderStageFlagBits::eCompute;
		stage.module = shader_module;
		stage.pName = shaderEntryPoint;

		reflection_builder.addShaderStage(vk::ShaderStageFlagBits::eCompute, *code.code);
	}

	auto reflection = std::make_unique<FVkShaderReflectionData>(reflection_builder.build());

	// Update shader stage specialization info
	for (auto& shader_stage : pipeline_stages)
	{
		auto found = reflection->specializationConstantStages.find(shader_stage.stage);
		if (found != reflection->specializationConstantStages.end())
			shader_stage.pSpecializationInfo = &found->second.info;
	}

	shader->reflection = m_pResources->add<FVkShaderReflectionData>(name.c_str(), std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	createShaderResources(resources_info, *shader);

	FVkComputePipelineCreateInfo pipeline_ci{};
	pipeline_ci.layout = shader->layout;

	for (size_t idx = 0ull; idx < pipeline_stages.size(); ++idx)
	{
		auto& stage = pipeline_stages.at(idx);
		pipeline_ci.shaderStage = stage;
#ifndef BUILD_WITHOUT_DEBUGGIND
		pipeline_ci.name = std::format("{} #{}", name, idx);
#endif

		auto& pipeline = shader->handles.emplace_back();
		m_pDevice->createPipeline(pipeline_ci, pipeline);
	}

	shader->bindPoint = vk::PipelineBindPoint::eCompute;

	std::stringstream ss{};
	ss << name;
	for (const auto& handle : shader->handles)
		ss << ":" << std::hex << reinterpret_cast<uint64_t>(static_cast<vk::Pipeline::CType>(handle));

	auto shader_unique_name = ss.str();
	return m_pResources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader));
}

FShaderReflectionData CVkRenderContext::getShaderReflection(const size_t& shader_view)
{
	auto& shader = m_pResources->get<FVkShader>(shader_view);
	auto& shader_reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);
	FShaderReflectionData reflection_data{};

	for (const auto& [id, vkuniform] : shader_reflection->uniforms)
	{
		auto& uniform = reflection_data.uniforms[id];
		uniform.binding = vkuniform.binding;
		uniform.set = vkuniform.set;
		uniform.offset = vkuniform.offset;
		uniform.size = vkuniform.size;
	}

	for (const auto& [aid, vkuniform_block] : shader_reflection->uniformBlocks)
	{
		if (vkuniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
			continue;

		auto& uniform_block = reflection_data.uniformBlocks[aid];
		uniform_block.binding = vkuniform_block.binding;
		uniform_block.set = vkuniform_block.set;
		uniform_block.size = vkuniform_block.size;

		for (const auto& [bid, vkuniform] : vkuniform_block.uniforms)
		{
			auto& uniform = uniform_block.uniforms[bid];
			uniform.binding = vkuniform.binding;
			uniform.set = vkuniform.set;
			uniform.offset = vkuniform.offset;
			uniform.size = vkuniform.size;
		}
	}

	for (const auto& [aid, vkpush_block] : shader_reflection->pushBlocks)
	{
		auto& push_block = reflection_data.pushBlocks[aid];
		push_block.size = vkpush_block.size;

		for (const auto& [bid, vkuniform] : vkpush_block.uniforms)
		{
			auto& uniform = push_block.uniforms[bid];
			uniform.binding = vkuniform.binding;
			uniform.set = vkuniform.set;
			uniform.offset = vkuniform.offset;
			uniform.size = vkuniform.size;
		}
	}

	if(!shader_reflection->localSizes.empty())
		reflection_data.localSizes = shader_reflection->localSizes.front();

	return reflection_data;
}

// TODO: I only need to write data to descriptors if I have not written anything yet for the current 
// swapchain frame and if the previous hash sum of data does not match the current one. Otherwise, 
// I'll just be doing unnecessary work both copying data and loading it, including loading descriptors.
void CVkRenderContext::updateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags)
{
	auto& thread_context = m_perThreadContext[std::this_thread::get_id()];

	auto& shader = m_pResources->get<FVkShader>(shader_resources.shaderId);
	auto& shader_reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);

	auto& textures = m_pResources->data<FVkTexture>();
	auto& buffers = m_pResources->data<FVkBuffer>();

	std::unordered_set<uint32_t> updated_descriptor_sets{};
	std::array<vk::WriteDescriptorSet, 64ull> write_descriptor_sets{};
	std::array<vk::DescriptorImageInfo, 64ull> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, 64ull> buffer_descriptors{};

	size_t texture_count{ 0ull };
	size_t buffer_count{ 0ull };
	size_t writes_count{ 0ull };

	// TODO: add something like check sum to check changed data in buffers or not
	if (flags & EShaderResourcesUpdateFlagBits::eUniformBlocks)
	{
		for (const auto& [uniform_name, uniform_data] : shader_resources.uniforms)
		{
			auto found_reflection = shader_reflection->uniformBlocks.find(uniform_name);
			if (found_reflection == shader_reflection->uniformBlocks.end())
				continue;

			// Prepare descriptor update data
			auto& uniform_block = found_reflection->second;
			auto descriptor_index = shader->descriptorUsage[uniform_block.set];
			updated_descriptor_sets.emplace(uniform_block.set);

			auto buffer_id = shader->uniformBuffers[uniform_name];
			auto& buffer = buffers.at(buffer_id).front();
			auto buffer_element_size = buffer->size / buffer->instanceCount;

			// Not sure that it is correct, because i don't know how to determine previous buffer
			auto& buffer_descriptor = buffer_descriptors.at(buffer_count);
			buffer_descriptor.buffer = buffer->handle;
			buffer_descriptor.range = uniform_block.size;
			buffer_descriptor.offset = descriptor_index * buffer_element_size;
			buffer_count++;

			auto& current_descriptor_set = shader->descriptorSets[uniform_block.set].handles.at(descriptor_index);

			auto& write_descriptor_set = write_descriptor_sets.at(writes_count);
			write_descriptor_set.descriptorType = uniform_block.descriptorType;
			write_descriptor_set.dstBinding = uniform_block.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pBufferInfo = &buffer_descriptor;
			write_descriptor_set.descriptorCount = 1;
			writes_count++;

			// Write new data to buffer memory
			void* mapped_memory{ nullptr };
			m_pDevice->mapBufferMemory(*buffer, &mapped_memory);
			std::memcpy((char*)mapped_memory + buffer_descriptor.offset, uniform_data.data, uniform_data.size);
			m_pDevice->unmapBufferMemory(*buffer);
		}
	}

	// Bind storage buffers
	if (flags & EShaderResourcesUpdateFlagBits::eStorageBlocks)
	{
		for (const auto& [uniform_name, buffer_id] : shader_resources.buffers)
		{
			auto found_reflection = shader_reflection->uniformBlocks.find(uniform_name);
			if (found_reflection == shader_reflection->uniformBlocks.end())
				continue;

			// Prepare descriptor update data
			auto& uniform_block = found_reflection->second;
			updated_descriptor_sets.emplace(uniform_block.set);

			auto& buffer = buffers.at(buffer_id).front();

			// Not sure that it is correct, because i don't know how to determine previous buffer
			auto& buffer_descriptor = buffer_descriptors.at(buffer_count);
			buffer_descriptor.buffer = buffer->handle;
			buffer_descriptor.range = buffer->size;
			buffer_descriptor.offset = 0ull;
			buffer_count++;

			auto descriptor_index = shader->descriptorUsage[uniform_block.set];
			auto& current_descriptor_set = shader->descriptorSets[uniform_block.set].handles.at(descriptor_index);

			auto& write_descriptor_set = write_descriptor_sets.at(writes_count);
			write_descriptor_set.descriptorType = uniform_block.descriptorType;
			write_descriptor_set.dstBinding = uniform_block.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pBufferInfo = &buffer_descriptor;
			write_descriptor_set.descriptorCount = 1;
			writes_count++;
		}
	}

	// Updating textures in shader
	if (flags & EShaderResourcesUpdateFlagBits::eTextures)
	{
		for (const auto& [texture_name, bind_info] : shader_resources.textures)
		{
			size_t texture_index{ 0ull };
			if (textures.at(bind_info.textureId).size() == m_pDevice->m_swapchain.framesInFlight)
				texture_index = m_pDevice->m_swapchain.currentFrame;

			auto& texture = textures.at(bind_info.textureId).at(texture_index);

			auto found_reflection = shader_reflection->uniforms.find(texture_name);
			if (found_reflection == shader_reflection->uniforms.end())
				continue;

			auto& uniform = found_reflection->second;
			updated_descriptor_sets.emplace(uniform.set);

			// TODO: add image view selection support
			auto& image_descriptor = image_descriptors.at(texture_count);
			image_descriptor.imageView = texture->imageViews.at(bind_info.mipLevel);
			image_descriptor.sampler = texture->sampler;
			image_descriptor.imageLayout = texture->layout;
			texture_count++;

			auto descriptor_index = shader->descriptorUsage[uniform.set];
			auto& current_descriptor_set = shader->descriptorSets[uniform.set].handles.at(descriptor_index);

			auto& write_descriptor_set = write_descriptor_sets.at(writes_count);
			write_descriptor_set.descriptorType = uniform.descriptorType;
			write_descriptor_set.dstBinding = uniform.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pImageInfo = &image_descriptor;
			write_descriptor_set.descriptorCount = 1;
			writes_count++;
		}
	}

	// NOTE: here we updating index for descriptor set that we will bind in future
	for (auto& descriptor_set : updated_descriptor_sets)
	{
		auto usages = shader->descriptorUsage[descriptor_set];
		shader->descriptorUsage[descriptor_set] = (usages + 1u) % shader->maximumDescriptorSlots;
	}

	// Update descriptor sets if needed
	if (!write_descriptor_sets.empty())
	{
		auto& logical = m_pDevice->getLogicalDevice();
		logical.updateDescriptorSets(static_cast<uint32_t>(writes_count), write_descriptor_sets.data(), 0, nullptr);
	}
}

void CVkRenderContext::flushConstantRanges(size_t shader_id, const std::unordered_map<size_t, FUniformDataView>& constants, EQueueTypeFlagBits queue_type)
{
	auto& shader = m_pResources->get<FVkShader>(shader_id);
	auto& shader_reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);

	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	// TODO: remove all other posibilities to have more than one push block. 
	for (const auto& [constant_name, constant_data] : constants)
	{
		auto found_reflection = shader_reflection->pushBlocks.find(constant_name);
		if (found_reflection == shader_reflection->pushBlocks.end())
			continue;

		auto& constant_block = found_reflection->second;
		current.pushConstants(shader->layout, constant_block.stageFlags, 0, constant_data.size, constant_data.data);

		// Break, because we can have only one push constants block
		break;
	}
}

void CVkRenderContext::bindShader(size_t shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& shader = m_pResources->get<FVkShader>(shader_id);

	// TODO: make possible to select what pipeline i have to bind now
	current.bindPipeline(shader->bindPoint, shader->handles.back());

	bindShaderDescriptors(command_buffer, *shader);
}

void CVkRenderContext::destroyShader(size_t resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eShader });
}

size_t CVkRenderContext::createBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto& physical = m_pDevice->getPhysicalDevice();

	FVkBufferCreateInfo vk_buffer_ci{};
	vk_buffer_ci.instanceCount = buffer_ci.count;
	vk_buffer_ci.instanceSize = buffer_ci.size;
	
#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_buffer_ci.name = name;
#endif

	switch (buffer_ci.flags)
	{
	case EBufferTypeFlagBits::eStaging: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuOnly;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
	} break;
	case EBufferTypeFlagBits::eVertex: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eUnknown;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
	} break;
	case EBufferTypeFlagBits::eIndex: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eUnknown;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer;
	} break;
	case EBufferTypeFlagBits::eStorage: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer;
	} break;
	case EBufferTypeFlagBits::eUniform: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;
		vk_buffer_ci.minOffsetAlignment = std::lcm(physical.properties.limits.minUniformBufferOffsetAlignment, physical.properties.limits.nonCoherentAtomSize);
	} break;
	case EBufferTypeFlagBits::eIndirect: {
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
	} break;
	default:
		break;
	}

	auto buffer = std::make_unique<FVkBuffer>();
	m_pDevice->createBuffer(vk_buffer_ci, *buffer);

	auto buffer_unique_name = create_object_unique_name("buffer:{:x}:{}", buffer->handle, name);
	auto buffer_id = m_pResources->add<FVkBuffer>(buffer_unique_name.c_str(), std::move(buffer));

	if (!buffer_ci.upload.data.empty())
	{
		auto& upload_info = m_bufferUploadInfos.emplace_back();
		upload_info.bufferId = buffer_id;
		upload_info.count = buffer_ci.count;
		upload_info.size = buffer_ci.size;
		upload_info.offset = buffer_ci.upload.offset;
		upload_info.data = buffer_ci.upload.data;
	}

	return buffer_id;
}

void CVkRenderContext::uploadDataToBuffer(const FBufferUploadInfo& upload_info)
{
	m_bufferUploadInfos.emplace_back(upload_info);
}

FBufferView CVkRenderContext::getBufferView(size_t buffer_id)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	return { buffer->size, buffer->instanceCount, buffer->address };
}

void CVkRenderContext::bindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	std::vector<vk::Buffer> buffers{};
	for (const auto& buffer_id : binding_info.buffers)
		buffers.emplace_back(m_pResources->get<FVkBuffer>(buffer_id)->handle);

	current.bindVertexBuffers(binding_info.firstBinding, buffers, binding_info.offsets);
}

void CVkRenderContext::bindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	vk::IndexType index_type{ vk::IndexType::eNoneKHR };
	switch (binding_info.indexSize)
	{
	case 4: index_type = vk::IndexType::eUint32; break;
	case 2: index_type = vk::IndexType::eUint16; break;
	case 1: index_type = vk::IndexType::eUint8EXT; break;
	}

	auto& buffer = m_pResources->get<FVkBuffer>(binding_info.bufferId);
	current.bindIndexBuffer(buffer->handle, binding_info.offset, index_type);
}

void CVkRenderContext::mapBufferMemory(size_t buffer_id, void*& memory)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->mapBufferMemory(*buffer, &memory);
}

void CVkRenderContext::unmapBufferMemory(size_t buffer_id)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->unmapBufferMemory(*buffer);
}

void CVkRenderContext::flushBuffer(size_t buffer_id, size_t size, size_t offset)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->flushBufferMemory(*buffer, size, offset);
}

void CVkRenderContext::destroyBuffer(size_t resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eBuffer });
}

void CVkRenderContext::pushG2CBarriers(EQueueTypeFlagBits queue_type, const std::vector<size_t>& texture_ids)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures.at(texture_id);
		auto& current = texture.at(m_pDevice->m_swapchain.currentFrame);

		auto& image_trans_info = translation_info.imageTranslationInfos.emplace_back();
		image_trans_info.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		image_trans_info.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
		image_trans_info.oldLayout = vk::ImageLayout::eUndefined;
		image_trans_info.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
		image_trans_info.newLayout = get_render_target_final_layout(*current);
		image_trans_info.handle = current->image.handle;

		image_trans_info.subresourceRange.aspectMask = get_texture_aspect_flags(*current);
		image_trans_info.subresourceRange.baseArrayLayer = 0;
		image_trans_info.subresourceRange.layerCount = current->totalLayers;
		image_trans_info.subresourceRange.baseMipLevel = 0;
		image_trans_info.subresourceRange.levelCount = current->initialData.base.mipLevels;
	}

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::pushC2CBarrier(EQueueTypeFlagBits queue_type, const std::vector<size_t>& texture_ids)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures.at(texture_id);
		auto& current = texture.at(m_pDevice->m_swapchain.currentFrame);
	
		auto& image_trans_info = translation_info.imageTranslationInfos.emplace_back();
		image_trans_info.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.srcAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eMemoryWrite;
		image_trans_info.oldLayout = get_render_target_final_layout(*current);
		image_trans_info.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderSampledRead | vk::AccessFlagBits2::eMemoryRead;
		image_trans_info.newLayout = get_render_target_final_layout(*current);
		image_trans_info.handle = current->image.handle;
	
		image_trans_info.subresourceRange.aspectMask = get_texture_aspect_flags(*current);
		image_trans_info.subresourceRange.baseArrayLayer = 0;
		image_trans_info.subresourceRange.layerCount = current->totalLayers;
		image_trans_info.subresourceRange.baseMipLevel = 0;
		image_trans_info.subresourceRange.levelCount = current->initialData.base.mipLevels;
	}

	//FVkTranslationInfo translation_info{};
	//auto& barrier = translation_info.memoryTranslationInfos.emplace_back();
	//barrier.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	//barrier.srcAccessMask = vk::AccessFlagBits2::eShaderWrite;
	//barrier.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	//barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::pushC2GBarrier(EQueueTypeFlagBits queue_type, const std::vector<size_t>& texture_ids)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures.at(texture_id);
		auto& current = texture.at(m_pDevice->m_swapchain.currentFrame);
	
		auto& image_trans_info = translation_info.imageTranslationInfos.emplace_back();
		image_trans_info.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.srcAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eMemoryWrite;
		image_trans_info.oldLayout = get_render_target_final_layout(*current);
		image_trans_info.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
		image_trans_info.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderSampledRead;
		image_trans_info.newLayout = get_render_target_final_layout(*current);
		image_trans_info.handle = current->image.handle;
	
		image_trans_info.subresourceRange.aspectMask = get_texture_aspect_flags(*current);
		image_trans_info.subresourceRange.baseArrayLayer = 0;
		image_trans_info.subresourceRange.layerCount = current->totalLayers;
		image_trans_info.subresourceRange.baseMipLevel = 0;
		image_trans_info.subresourceRange.levelCount = current->initialData.base.mipLevels;
	}

	//auto& barrier = translation_info.memoryTranslationInfos.emplace_back();
	//barrier.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	//barrier.srcAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
	//barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
	//barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eMemoryRead;

	processMemoryBarriers(command_buffer, translation_info);
}


void CVkRenderContext::beginRendering(const FBeginRenderingInfo& rendering_info)
{
	// TODO: looks very bad. Maybe remove other method and do all work here?
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);

	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};

	std::array<vk::ClearValue, 3ull> clear_values{};
	clear_values[0].color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };
	clear_values[1].depthStencil = vk::ClearDepthStencilValue{ 1.f, 0u };
	clear_values[2].depthStencil = vk::ClearDepthStencilValue{ 1.f, 0u };

	// Prepare render targets
	std::vector<vk::RenderingAttachmentInfoKHR> color_attachments{};
	for (const auto& target_view : rendering_info.colorAttachments)
	{
		auto& render_target = textures.at(target_view).at(m_pDevice->m_swapchain.currentFrame);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		render_target->layout = texture_translation_info.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(*render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		auto& attachment_info = color_attachments.emplace_back();

		// TODO: here i binding first image view to rendering, 
		// but it is incorrect, because other image views can be 
		// mip levels, it means that current code can't draw in mip 
		// levels or array layers.
		attachment_info.imageView = render_target->imageViews.front();
		attachment_info.imageLayout = render_target->layout;
		attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
		attachment_info.loadOp = get_attachment_load_op(*render_target);
		attachment_info.storeOp = get_attachment_store_op(*render_target);
		attachment_info.clearValue = clear_values[0];
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if (rendering_info.depthAttachment != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment).at(m_pDevice->m_swapchain.currentFrame);

		bool is_depth_stencil_attachment = !is_depth_only_format(static_cast<vk::Format>(depth_target->initialData.base.format));

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		depth_target->layout = texture_translation_info.newLayout = is_depth_stencil_attachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimalKHR;
		auto target_layout_translation = get_texture_layout_translation_info(*depth_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
		
		depth_attachment.imageView = depth_target->imageViews.front();
		depth_attachment.imageLayout = depth_target->layout;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = get_attachment_load_op(*depth_target);
		depth_attachment.storeOp = get_attachment_store_op(*depth_target);
		depth_attachment.clearValue = clear_values[1];
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment != rendering_info.depthAttachment && rendering_info.stencilAttachment != invalid_handle)
	{
		auto& stencil_target = textures.at(rendering_info.stencilAttachment).at(m_pDevice->m_swapchain.currentFrame);

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		stencil_target->layout = texture_translation_info.newLayout = vk::ImageLayout::eStencilAttachmentOptimalKHR;
		auto target_layout_translation = get_texture_layout_translation_info(*stencil_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);

		stencil_attachment.imageView = stencil_target->imageViews.front();
		stencil_attachment.imageLayout = stencil_target->layout;
		stencil_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		stencil_attachment.loadOp = get_attachment_load_op(*stencil_target);
		stencil_attachment.storeOp = get_attachment_store_op(*stencil_target);
		stencil_attachment.clearValue = clear_values[2];
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
	begin_rendering_info.layerCount = rendering_info.layerCount;

	if (rendering_info.depthAttachment != invalid_handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures.at(rendering_info.depthAttachment).at(m_pDevice->m_swapchain.currentFrame);
		if (!is_depth_only_format(static_cast<vk::Format>(depth_target->initialData.base.format)) && rendering_info.stencilAttachment == rendering_info.depthAttachment)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment != invalid_handle && rendering_info.stencilAttachment != rendering_info.depthAttachment)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->debugBeginLabel(current_buffer, rendering_info.name, 0xffffffff);
#endif
	
	current_buffer.beginRenderingKHR(begin_rendering_info);
}

void CVkRenderContext::endRendering(const FBeginRenderingInfo& rendering_info)
{
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);
	
	auto& textures = m_pResources->data<FVkTexture>();

	auto& current_buffer = command_buffer.get();
	current_buffer.endRenderingKHR();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->endLabel(current_buffer);
#endif

	FVkTranslationInfo translation_info{};

	for (const auto& target_view : rendering_info.colorAttachments)
	{
		auto& render_target = textures.at(target_view).at(m_pDevice->m_swapchain.currentFrame);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		render_target->layout = texture_translation_info.newLayout = get_render_target_final_layout(*render_target);
		auto target_layout_translation = get_texture_layout_translation_info(*render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	if (rendering_info.depthAttachment != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment).at(m_pDevice->m_swapchain.currentFrame);

		bool is_depth_stencil_attachment = !is_depth_only_format(static_cast<vk::Format>(depth_target->initialData.base.format));

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = is_depth_stencil_attachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eDepthAttachmentOptimal;
		depth_target->layout = texture_translation_info.newLayout = get_render_target_final_layout(*depth_target);
		auto target_layout_translation = get_texture_layout_translation_info(*depth_target, texture_translation_info);
		//target_layout_translation.subresourceRange.aspectMask = is_depth_stencil_attachment ? vk::ImageAspectFlagBits::eDepth : target_layout_translation.subresourceRange.aspectMask;
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	if (rendering_info.stencilAttachment != invalid_handle && rendering_info.depthAttachment != rendering_info.stencilAttachment)
	{
		auto& stencil_target = textures.at(rendering_info.stencilAttachment).at(m_pDevice->m_swapchain.currentFrame);;

		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eStencilAttachmentOptimal;
		stencil_target->layout = texture_translation_info.newLayout = get_render_target_final_layout(*stencil_target);
		auto target_layout_translation = get_texture_layout_translation_info(*stencil_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	// Translate all render target layouts
	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::beginRendering2(const FBeginRendering2Info& rendering_info)
{
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);

	auto& textures = m_pResources->data<FVkTexture>();

	std::array<vk::ClearValue, 3ull> clear_values{};
	clear_values[0].color = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };
	clear_values[1].depthStencil = vk::ClearDepthStencilValue{ 1.f, 0u };
	clear_values[2].depthStencil = vk::ClearDepthStencilValue{ 1.f, 0u };

	// Prepare render targets
	std::vector<vk::RenderingAttachmentInfoKHR> color_attachments{};
	for (const auto& attachment_info : rendering_info.colorAttachments)
	{
		auto& render_target = textures.at(attachment_info.handle).at(m_pDevice->m_swapchain.currentFrame);

		auto& attachment_info = color_attachments.emplace_back();
		attachment_info.imageView = render_target->imageViews.front();
		attachment_info.imageLayout = render_target->layout;
		attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
		attachment_info.loadOp = get_attachment_load_op(*render_target);
		attachment_info.storeOp = get_attachment_store_op(*render_target);
		attachment_info.clearValue = clear_values[0];
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if (rendering_info.depthAttachment.handle != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment.handle).at(m_pDevice->m_swapchain.currentFrame);

		depth_attachment.imageView = depth_target->imageViews.front();
		depth_attachment.imageLayout = depth_target->layout;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = get_attachment_load_op(*depth_target);
		depth_attachment.storeOp = get_attachment_store_op(*depth_target);
		depth_attachment.clearValue = clear_values[1];
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment.handle != rendering_info.depthAttachment.handle && rendering_info.stencilAttachment.handle != invalid_handle)
	{
		auto& stencil_target = textures.at(rendering_info.stencilAttachment.handle).at(m_pDevice->m_swapchain.currentFrame);

		stencil_attachment.imageView = stencil_target->imageViews.front();
		stencil_attachment.imageLayout = stencil_target->layout;
		stencil_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		stencil_attachment.loadOp = get_attachment_load_op(*stencil_target);
		stencil_attachment.storeOp = get_attachment_store_op(*stencil_target);
		stencil_attachment.clearValue = clear_values[2];
	}

	vk::RenderingInfoKHR begin_rendering_info{};
	begin_rendering_info.renderArea.offset.x = rendering_info.renderArea.offset.x;
	begin_rendering_info.renderArea.offset.y = rendering_info.renderArea.offset.y;
	begin_rendering_info.renderArea.extent.width = rendering_info.renderArea.size.x;
	begin_rendering_info.renderArea.extent.height = rendering_info.renderArea.size.y;
	begin_rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
	begin_rendering_info.pColorAttachments = color_attachments.data();
	begin_rendering_info.layerCount = rendering_info.layerCount;

	if (rendering_info.depthAttachment.handle != invalid_handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures.at(rendering_info.depthAttachment.handle).at(m_pDevice->m_swapchain.currentFrame);
		if (!is_depth_only_format(static_cast<vk::Format>(depth_target->initialData.base.format)) && rendering_info.stencilAttachment.handle == rendering_info.depthAttachment.handle)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment.handle != invalid_handle && rendering_info.stencilAttachment.handle != rendering_info.depthAttachment.handle)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->debugBeginLabel(current_buffer, rendering_info.name, 0xffffffff);
#endif

	current_buffer.beginRenderingKHR(begin_rendering_info);
}

void CVkRenderContext::endRendering2(EQueueTypeFlagBits queue)
{
	auto& command_buffer = getLastCommandBuffer(queue);

	auto& textures = m_pResources->data<FVkTexture>();

	auto& current_buffer = command_buffer.get();
	current_buffer.endRenderingKHR();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->endLabel(current_buffer);
#endif
}

void CVkRenderContext::beginCompute(const FBeginComputeInfo& rendering_info)
{
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);
	auto& textures = m_pResources->data<FVkTexture>();
	auto& buffers = m_pResources->data<FVkBuffer>();

	FVkTranslationInfo translation_info{};

	// Translate storage images to general layout (uav)
	for (const auto& target_view : rendering_info.writeAttachments)
	{
		auto& render_target = textures.at(target_view).at(m_pDevice->m_swapchain.currentFrame);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
		render_target->layout = texture_translation_info.newLayout = vk::ImageLayout::eGeneral;
		auto target_layout_translation = get_texture_layout_translation_info(*render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	for (const auto& target_view : rendering_info.writeBuffers)
	{
		auto& buffer = buffers.at(target_view).at(0);

		FVkBufferTranslationInfo buffer_translation_info{};
		buffer_translation_info.handle = buffer->handle;
		buffer_translation_info.size = buffer->size;
		buffer_translation_info.srcStageMask = vk::PipelineStageFlagBits2KHR::eComputeShader;
		buffer_translation_info.srcAccessMask = vk::AccessFlagBits2KHR::eMemoryWrite | vk::AccessFlagBits2KHR::eShaderWrite;
		buffer_translation_info.dstStageMask = vk::PipelineStageFlagBits2KHR::eComputeShader | vk::PipelineStageFlagBits2KHR::eVertexShader | vk::PipelineStageFlagBits2KHR::eFragmentShader;
		buffer_translation_info.dstAccessMask = vk::AccessFlagBits2KHR::eMemoryRead | vk::AccessFlagBits2KHR::eShaderRead;
		translation_info.bufferTranslationInfos.emplace_back(buffer_translation_info);
	}

	processMemoryBarriers(command_buffer, translation_info);

#ifndef BUILD_WITHOUT_DEBUGGIND
	auto& current_buffer = command_buffer.get();
	m_pDevice->debugBeginLabel(current_buffer, rendering_info.name, 0x888888ff);
#endif
}

void CVkRenderContext::endCompute(const FBeginComputeInfo& rendering_info)
{
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);
	auto& textures = m_pResources->data<FVkTexture>();

#ifndef BUILD_WITHOUT_DEBUGGIND
	auto& current_buffer = command_buffer.get();
	m_pDevice->endLabel(current_buffer);
#endif

	FVkTranslationInfo translation_info{};

	// Translate storage images to general layout (uav)
	for (const auto& target_view : rendering_info.writeAttachments)
	{
		auto& render_target = textures.at(target_view).at(m_pDevice->m_swapchain.currentFrame);

		// Translating render target layout to color attachment
		FVkTextureTranslationInfo texture_translation_info{};
		texture_translation_info.oldLayout = vk::ImageLayout::eGeneral;
		render_target->layout = texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		auto target_layout_translation = get_texture_layout_translation_info(*render_target, texture_translation_info);
		translation_info.imageTranslationInfos.emplace_back(target_layout_translation);
	}

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info)
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

	auto& current_buffer = command_buffer.get();
	current_buffer.pipelineBarrier2KHR(dependency_info);
}

void CVkRenderContext::setViewport(float width, float height, bool flip_viewport)
{
	vk::Viewport viewport{};
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	if (flip_viewport)
	{
		viewport.width = width;
		viewport.height = -height;
		viewport.x = 0.f;
		viewport.y = height;
	}
	else
	{
		viewport.width = width;
		viewport.height = height;
		viewport.x = 0.f;
		viewport.y = 0.f;
	}

	// Because i can set family only in graphics
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	auto& current = command_buffer.get();

	current.setViewport(0, 1, &viewport);
}

void CVkRenderContext::setScissorRect(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height)
{
	vk::Rect2D scissor_rect{};
	scissor_rect.offset.x = offset_x;
	scissor_rect.offset.y = offset_y;
	scissor_rect.extent.width = width;
	scissor_rect.extent.height = height;

	// Because i can set family only in graphics
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	auto& current = command_buffer.get();

	current.setScissor(0, 1, &scissor_rect);
}

// TODO: add indirect draw support
void CVkRenderContext::draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	if (draw_info.lastIndex == 0ull && draw_info.lastVertex == 0ull)
	{
		auto& range = draw_info.ranges.front();
		current.draw(range.vertexCount, 1, range.beginVertex, 0);
	}
	else if (draw_info.lastIndex == 0ull)
	{
		for (auto& range : draw_info.ranges)
		{
			auto vertex_count = range.vertexCount == 0ull ? draw_info.lastIndex : range.vertexCount;
			current.draw(vertex_count, 1, range.beginVertex, 0);
		}
	}
	else
	{
		for (auto& range : draw_info.ranges)
		{
			auto index_count = range.indexCount == 0ull ? draw_info.lastIndex : range.indexCount;
			current.drawIndexed(index_count, 1, range.beginIndex, 0, 0);
		}
	}
}

void CVkRenderContext::draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& indirect_command_buffer = m_pResources->get<FVkBuffer>(draw_info.bufferId);
	current.drawIndexedIndirect(indirect_command_buffer->handle, draw_info.offset, draw_info.count, draw_info.stride);
}

void CVkRenderContext::dispatch(EQueueTypeFlagBits queue_type, const FDispatchInfo& dispatch_info)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	bool use_indirect_dispatch{ dispatch_info.groups.size() > 1ull };

	std::vector<vk::DispatchIndirectCommand> commands{};
	for (const auto& group : dispatch_info.groups)
	{
		auto& command = commands.emplace_back();
		command.x = group.x;
		command.y = group.y;
		command.z = group.z;
	}

	if (use_indirect_dispatch)
	{
		void* memory{ nullptr };

		auto& buffer = m_pResources->get<FVkBuffer>(m_indirectDispatchBuffer);
		m_pDevice->mapBufferMemory(*buffer, &memory);
		std::memcpy(static_cast<char*>(memory) + m_dispatchBufferOffset, commands.data(), commands.size() * sizeof(vk::DispatchIndirectCommand));
		m_pDevice->unmapBufferMemory(*buffer);
		current.dispatchIndirect(buffer->handle, m_dispatchBufferOffset);
	}
	else
	{
		auto& command = commands.front();
		current.dispatch(command.x, command.y, command.z);
	}
}

void CVkRenderContext::dispatch2(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_info)
{
	auto& shader = m_pResources->get<FVkShader>(dispatch_info.shaderHandle);
	auto& reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);

	vk::DispatchIndirectCommand command{};
	command.x = dispatch_info.globalSizes.x;
	command.y = dispatch_info.globalSizes.y;
	command.z = dispatch_info.globalSizes.z;

	if (dispatch_info.divideByLocals && !reflection->localSizes.empty())
	{
		auto& localSizes = reflection->localSizes.front();
		command.x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(command.x) / static_cast<float>(localSizes[0]))), 1u);
		command.y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(command.y) / static_cast<float>(localSizes[1]))), 1u);
		command.z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(command.z) / static_cast<float>(localSizes[2]))), 1u);
	}

	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();
	current.dispatch(command.x, command.y, command.z);
}

void CVkRenderContext::dispatch2(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info)
{
	
}

size_t CVkRenderContext::getTextureResource(const char* name)
{
	return m_pResources->getId<FVkTexture>(name);
}

size_t CVkRenderContext::getShaderResource(const char* name)
{
	return m_pResources->getId<FVkShader>(name);
}

FPoint2D<uint32_t> CVkRenderContext::getRenderResolution()
{
	FPoint2D<uint32_t> resolution{};
	resolution.x = m_pDevice->m_swapchain.extent.width;
	resolution.y = m_pDevice->m_swapchain.extent.height;

	return resolution;
}

FRect2D<float> CVkRenderContext::getRenderRectFromRT(size_t texture_id)
{
	auto& texture = m_pResources->get<FVkTexture>(texture_id);

	FRect2D<float> rect{};
	rect.size.x = static_cast<float>(texture->initialData.base.width);
	rect.size.y = static_cast<float>(texture->initialData.base.height);

	return rect;
}

FRect2D<float> CVkRenderContext::getScreenRenderRect()
{
	FRect2D<float> rect{};
	rect.size.x = static_cast<float>(m_pDevice->m_swapchain.extent.width);
	rect.size.y = static_cast<float>(m_pDevice->m_swapchain.extent.height);
	return rect;
}

uint32_t CVkRenderContext::getDepthFormat(bool depth_only)
{
	auto found_format = m_pDevice->findSupportedFormat
	(
		{
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD16Unorm
		},
		depth_only
	);

	return static_cast<uint32_t>(found_format);
}

uint32_t CVkRenderContext::getApiFormat(EApiDataFormat global_format)
{
	switch (global_format)
	{
	case EApiDataFormat::eRGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB; break;

	case EApiDataFormat::eR8_UNORM: return VK_FORMAT_R8_UNORM; break;
	case EApiDataFormat::eRG8_UNORM: return VK_FORMAT_R8G8_UNORM; break;
	case EApiDataFormat::eRGB8_UNORM: return VK_FORMAT_R8G8B8_UNORM; break;
	case EApiDataFormat::eRGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM; break;

	case EApiDataFormat::eR16_UNORM: return VK_FORMAT_R16_UNORM; break;
	case EApiDataFormat::eRG16_UNORM: return VK_FORMAT_R16G16_UNORM; break;
	case EApiDataFormat::eRGB16_UNORM: return VK_FORMAT_R16G16B16_UNORM; break;
	case EApiDataFormat::eRGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM; break;

	case EApiDataFormat::eR32_UINT: return VK_FORMAT_R32_UINT; break;
	case EApiDataFormat::eRG32_UINT: return VK_FORMAT_R32G32_UINT; break;
	case EApiDataFormat::eRGB32_UINT: return VK_FORMAT_R32G32B32_UINT; break;
	case EApiDataFormat::eRGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT; break;

	case EApiDataFormat::eR16_SFLOAT: return VK_FORMAT_R16_SFLOAT; break;
	case EApiDataFormat::eRG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT; break;
	case EApiDataFormat::eRGB16_SFLOAT: return VK_FORMAT_R16G16B16_SFLOAT; break;
	case EApiDataFormat::eRGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT; break;

	case EApiDataFormat::eR32_SFLOAT: return VK_FORMAT_R32_SFLOAT; break;
	case EApiDataFormat::eRG32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT; break;
	case EApiDataFormat::eRGB32_SFLOAT: return VK_FORMAT_R32G32B32_SFLOAT; break;
	case EApiDataFormat::eRGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT; break;
	default: return 0; break;
	}
}

uint32_t CVkRenderContext::getPresentFormat()
{
	return static_cast<uint32_t>(m_pDevice->m_swapchain.format);
}

// TODO: update only targets, that have same size with swapchain
void CVkRenderContext::updateSurface(int32_t surface_width, int32_t surface_height)
{
	if (this->m_iSurfaceWidth != surface_width || this->m_iSurfaceHeight != surface_height)
	{
		this->m_iSurfaceWidth = surface_width;
		this->m_iSurfaceHeight = surface_height;

		auto& logical_device = m_pDevice->getLogicalDevice();
		logical_device.waitIdle();

		m_pDevice->cleanupSwapchain();
		m_pDevice->createSwapchain(surface_width, surface_height);
		m_pDevice->m_swapchain.currentFrame = 0u;

		auto& textures = m_pResources->data<FVkTexture>();

		// Updating render targets
		
		for (size_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
		{
			bool is_storage{ false };
			size_t swapchainFrameIndex{ 0ull };
			for (auto& target : textures.at(texture_id))
			{
				if (!target->isRenderTarget || (target->initialData.base.sizePolicy == ERenderTargetSizePolicy::eFixed))
					continue;

				// Update sapchain target
				if (target->initialData.base.isSwapchainTarget)
				{
					target->imageViews.clear();
					target->image.handle = m_pDevice->m_swapchain.images.at(swapchainFrameIndex);
					target->imageViews.emplace_back(m_pDevice->m_swapchain.imageViews.at(swapchainFrameIndex));
					target->initialData.base.width = surface_width;
					target->initialData.base.height = surface_height;

					swapchainFrameIndex++;
					continue;
				}

				for (auto& image_view : target->imageViews)
					m_pDevice->destroyImageView(image_view);
				target->imageViews.clear();

				m_pDevice->destroyImage(target->image);

				FVkTextureCreateInfo texture_ci{};
				texture_ci.base = target->initialData.base;
				texture_ci.createSampler = false;
#ifndef BUILD_WITHOUT_DEBUGGIND
				texture_ci.name = target->name;
#endif
				// TODO: here i should set rendering resolution, not swapchain image size
				texture_ci.base.width = surface_width;
				texture_ci.base.height = surface_height;

				check_api_result(createTexture(texture_ci, *target));
				
				is_storage = utl::cast<bool>(texture_ci.base.typeFlags & ETextureCreateFlagBits::eStorage);
			}

			//if (is_storage)
			//	m_textureLayoutInfos.emplace_back(texture_id);
		}
	}
}

FVkCommandBuffer& CVkRenderContext::createCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[command_buffer_ci.mask];

	if (!per_queue.commandPool.handle)
	{
		FVkCommandPoolCreateInfo pool_ci{};
		pool_ci.mask = command_buffer_ci.mask;

		check_api_result(m_pDevice->createCommandPool(pool_ci, per_queue.commandPool));
	}
	
	auto& command_buffer = per_queue.commandBuffers.emplace_back();
	check_api_result(m_pDevice->createCommandBuffer(command_buffer_ci, per_queue.commandPool, command_buffer));

	return command_buffer;
}

FVkCommandBuffer& CVkRenderContext::getLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers.back();
}

FVkCommandBuffer& CVkRenderContext::getCommandBuffer(EQueueTypeFlagBits queue_type, size_t index, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers.at(index);
}

std::vector<FVkCommandBuffer>& CVkRenderContext::getCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers;
}

void CVkRenderContext::destroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	auto& command_buffer = per_queue.commandBuffers.back();
	m_pDevice->destroyCommandBuffer(per_queue.commandPool, command_buffer);
	per_queue.commandBuffers.pop_back();
}

void CVkRenderContext::beginFrameCommandBuffers()
{
	uint32_t image_index{ 0u };

	processResourceBarriers();
	processResourceUpload();
	// TODO: sync queues here

	vk::Result res{};
	try { res = m_pDevice->acquireNextImage(image_index); }
	catch (vk::OutOfDateKHRError error) { res = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("Failed to acquire swap chain image!"); }

	if (res == vk::Result::eErrorOutOfDateKHR)
		updateSurface(m_iSurfaceWidth, m_iSurfaceHeight);
	else if (res == vk::Result::eSuboptimalKHR)
		log_error("Failed to acquire swap chain image!");

	if (res != vk::Result::eSuccess)
		return;

	m_bIsFrameStarted = true;

	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	m_pDevice->beginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eRenderPassContinue, image_index);
}

void CVkRenderContext::endFrameCommandBuffers()
{
	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	m_pDevice->endCommandBuffer(command_buffer);
	m_bIsFrameStarted = false;

	vk::Result presentResult{};
	try { presentResult = m_pDevice->present(command_buffer); }
	catch (vk::OutOfDateKHRError err) { presentResult = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("failed to present swap chain image!"); }

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		updateSurface(m_iSurfaceWidth, m_iSurfaceHeight);

	// Destroy unused contexts
	for (auto it = m_perThreadContext.begin(); it != m_perThreadContext.end();)
	{
		if (it->second.use_count() < 1)
		{
			destroyThreadContext(it->second);
			it = m_perThreadContext.erase(it);
			continue;
		}
		++it;
	}

	// Try to free resources
	if (m_utilizationCounter % m_pDevice->m_swapchain.framesInFlight == 0)
		processResourceUtilization();

	// Frame finished. Textures in now again in initial layout
	auto& textures = m_pResources->data<FVkTexture>();
	for (auto& texture_instances : textures)
	{
		for (auto& texture : texture_instances)
		{
			if (texture->isRenderTarget && !(texture->initialData.base.typeFlags & ETextureCreateFlagBits::eStorage))
				texture->layout = vk::ImageLayout::eUndefined;
		}
	}

	m_utilizationCounter++;
}

bool CVkRenderContext::isSwapChainFinished()
{
	return m_utilizationCounter % m_pDevice->m_swapchain.framesInFlight == 0;
}

void CVkRenderContext::beginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, uint32_t color)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->debugBeginLabel(current, label, color);
}

void CVkRenderContext::insertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, uint32_t color)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->debugInsertLabel(current, label, color);
}

void CVkRenderContext::endDebugLabel(EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->endLabel(current);
}

void CVkRenderContext::destroyThreadContext(FVkThreadContext& context)
{
	// Destroy command buffers and pools
	for (auto& [queue_id, queue_resources] : context.perQueue)
	{
		for (auto& command_buffer : queue_resources.commandBuffers)
			m_pDevice->destroyCommandBuffer(queue_resources.commandPool, command_buffer);

		m_pDevice->destroyCommandPool(queue_resources.commandPool);
	}

	// Destroy descriptor sets
	for (auto& [id, descriptor_set] : context.descriptorSets)
		m_pDevice->destroyDescriptorSet(context.descriptorPool, descriptor_set);

	// Destroy descriptor pool
	m_pDevice->destroyDescriptorPool(context.descriptorPool);
}

void CVkRenderContext::processResourceBarriers()
{
	if (m_textureLayoutInfos.empty())
		return;

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1u;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "resource_barriers";
#endif

	auto& textures = m_pResources->data<FVkTexture>();

	auto& command_buffer = createCommandBuffer(command_buffer_ci);

	m_pDevice->beginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	FVkTranslationInfo translation_info{};
	for (const auto& layout_info : m_textureLayoutInfos)
	{
		auto& texture = textures.at(layout_info.textureId);
		for (auto& current : texture)
		{
			if (current->initialData.base.typeFlags & ETextureCreateFlagBits::eStorage)
			{
				auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

				FVkTextureTranslationInfo texture_translation_info{};
				texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
				current->layout = texture_translation_info.newLayout = vk::ImageLayout::eGeneral;

				image_translation = get_texture_layout_translation_info(*current, texture_translation_info);
			}
		}
	}

	processMemoryBarriers(command_buffer, translation_info);

	m_pDevice->endCommandBuffer(command_buffer);
	m_pDevice->submitCommandBuffer(command_buffer);

	destroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

	m_textureLayoutInfos.clear();
}

void CVkRenderContext::processResourceUpload()
{
	if (m_textureUploadInfos.empty() && m_bufferUploadInfos.empty())
		return;

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1u;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "resource_upload";
#endif

	auto& textures = m_pResources->data<FVkTexture>();

	auto& command_buffer = createCommandBuffer(command_buffer_ci);

	m_pDevice->beginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	std::vector<FVkBuffer> staging_buffers{};

	// Upload textures
	{
		// Prepare staging buffers
		
		for (auto& texture_upload : m_textureUploadInfos)
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
			m_pDevice->createBuffer(staging_buffer_ci, staging_buffer);

			// Copy data to buffer
			void* mapped_buffer_memory{ nullptr };
			m_pDevice->mapBufferMemory(staging_buffer, &mapped_buffer_memory);
			std::memcpy(mapped_buffer_memory, texture_upload.data.data(), texture_upload.data.size());
			m_pDevice->unmapBufferMemory(staging_buffer);

			texture_upload.data.clear();
		}

		// Translate all images to transfer dst stage
		std::vector<size_t> generate_mip_textures{};
		FVkTranslationInfo translation_info{};
		for (const auto& texture_upload : m_textureUploadInfos)
		{
			auto& texture = textures.at(texture_upload.textureId).front();

			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
			texture->layout = texture_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);

			if (texture->initialData.base.mipLevels > 1u && texture_upload.mipLayerOffsets.empty())
				generate_mip_textures.emplace_back(texture_upload.textureId);
		}

		// Process layout transfers
		processMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();

		// Copy buffer to image
		for (size_t idx = 0ull; idx < m_textureUploadInfos.size(); ++idx)
		{
			auto& texture_upload = m_textureUploadInfos.at(idx);
			auto& texture = textures.at(texture_upload.textureId).front();
			auto& staging_buffer = staging_buffers.at(idx);

			auto aspect_mask = get_texture_aspect_flags(*texture);

			std::vector<vk::BufferImageCopy2KHR> copy_regions{};
			if (!texture_upload.mipLayerOffsets.empty())
			{
				for (uint32_t layer = 0u; layer < texture->totalLayers; ++layer)
				{
					auto& layer_offsets = texture_upload.mipLayerOffsets.at(layer);
					for (uint32_t mip = 0u; mip < texture->initialData.base.mipLevels; ++mip)
					{
						size_t offset = layer_offsets.at(mip);
						auto& copy_region = copy_regions.emplace_back();
						copy_region.imageSubresource.aspectMask = aspect_mask;
						copy_region.imageSubresource.mipLevel = mip;
						copy_region.imageSubresource.baseArrayLayer = layer;
						copy_region.imageSubresource.layerCount = texture->initialData.base.arrayLayers;
						copy_region.imageExtent.width = texture->initialData.base.width >> mip;
						copy_region.imageExtent.height = texture->initialData.base.height >> mip;
						copy_region.imageExtent.depth = texture->initialData.base.depth;
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
				copy_region.imageSubresource.layerCount = texture->totalLayers;
				copy_region.imageExtent.width = texture->initialData.base.width;
				copy_region.imageExtent.height = texture->initialData.base.height;
				copy_region.imageExtent.depth = texture->initialData.base.depth;
				copy_region.bufferOffset = 0ull;
			}

			copyBufferToTexture(command_buffer, staging_buffer, *texture, copy_regions);
		}

		auto& current_command_buffer = command_buffer.get();
		for (auto& texture_id : generate_mip_textures)
		{
			auto& texture = textures.at(texture_id).front();
			auto aspect_mask = get_texture_aspect_flags(*texture);

			uint32_t mip_width{ texture->initialData.base.width };
			uint32_t mip_heigth{ texture->initialData.base.height };

			FVkImageTranslationInfo image_translation_info{};
			image_translation_info.handle = texture->image.handle;
			image_translation_info.subresourceRange.baseArrayLayer = 0u;
			image_translation_info.subresourceRange.levelCount = 1;
			image_translation_info.subresourceRange.layerCount = texture->totalLayers;
			image_translation_info.subresourceRange.aspectMask = aspect_mask;

			for (uint32_t mip = 1u; mip < texture->initialData.base.mipLevels; ++mip)
			{
				image_translation_info.subresourceRange.baseMipLevel = mip - 1u;
				image_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				image_translation_info.srcStageMask = get_translation_stage_flags(image_translation_info.oldLayout);
				image_translation_info.srcAccessMask = get_translation_access_flags(image_translation_info.oldLayout);
				image_translation_info.newLayout = vk::ImageLayout::eTransferSrcOptimal;
				image_translation_info.dstStageMask = get_translation_stage_flags(image_translation_info.newLayout);
				image_translation_info.dstAccessMask = get_translation_access_flags(image_translation_info.newLayout);
				
				processImageLayoutTranslation(command_buffer, image_translation_info);

				vk::ImageBlit2KHR image_blit{};
				image_blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
				image_blit.srcOffsets[1] = vk::Offset3D{ static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_heigth), 1 };
				image_blit.srcSubresource.aspectMask = aspect_mask;
				image_blit.srcSubresource.mipLevel = image_translation_info.subresourceRange.baseMipLevel;
				image_blit.srcSubresource.baseArrayLayer = 0u;
				image_blit.srcSubresource.layerCount = 1u;
				image_blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
				image_blit.dstOffsets[1] = vk::Offset3D{ static_cast<int32_t>(mip_width > 1u ? mip_width / 2u : 1u), static_cast<int32_t>(mip_heigth > 1u ? mip_heigth / 2u : 1u), 1 };
				image_blit.dstSubresource.aspectMask = aspect_mask;
				image_blit.dstSubresource.mipLevel = mip;
				image_blit.dstSubresource.baseArrayLayer = 0u;
				image_blit.dstSubresource.layerCount = 1u;

				vk::BlitImageInfo2KHR blit_info{};
				blit_info.srcImage = texture->image.handle;
				blit_info.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
				blit_info.dstImage = texture->image.handle;
				blit_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
				blit_info.regionCount = 1u;
				blit_info.pRegions = &image_blit;
				blit_info.filter = vk::Filter::eNearest;

				current_command_buffer.blitImage2KHR(blit_info);

				image_translation_info.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
				image_translation_info.srcStageMask = get_translation_stage_flags(image_translation_info.oldLayout);
				image_translation_info.srcAccessMask = get_translation_access_flags(image_translation_info.oldLayout);
				image_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;
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
		for (const auto& texture_upload : m_textureUploadInfos)
		{
			auto& texture = textures.at(texture_upload.textureId).front();

			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			texture->layout = texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);
		}

		processMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();
	}

	// Upload buffers
	for(const auto& upload_info : m_bufferUploadInfos)
	{
		auto& physical = m_pDevice->getPhysicalDevice();

		FVkBufferCreateInfo vk_buffer_ci{};
		vk_buffer_ci.instanceCount = upload_info.count;
		vk_buffer_ci.instanceSize = upload_info.size;
		vk_buffer_ci.minOffsetAlignment = std::lcm(physical.properties.limits.minUniformBufferOffsetAlignment, physical.properties.limits.nonCoherentAtomSize);
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuOnly;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
#ifndef BUILD_WITHOUT_DEBUGGIND
		vk_buffer_ci.name = std::format("staging for #{}", upload_info.bufferId);
#endif

		auto src_buffer = staging_buffers.emplace_back();
		m_pDevice->createBuffer(vk_buffer_ci, src_buffer);

		// Copy data to staging buffer
		void* mapped_memory{ nullptr };
		m_pDevice->mapBufferMemory(src_buffer, &mapped_memory);

		char* dst_memory = static_cast<char*>(mapped_memory) + upload_info.offset;
		std::memcpy(dst_memory, upload_info.data.data(), upload_info.data.size());
		m_pDevice->unmapBufferMemory(src_buffer);

		auto& dst_buffer = m_pResources->get<FVkBuffer>(upload_info.bufferId);

		vk::BufferCopy2KHR copy_region{};
		copy_region.srcOffset = 0ull;
		copy_region.dstOffset = 0ull;
		copy_region.size = upload_info.data.size();

		vk::CopyBufferInfo2KHR copy_buffer{};
		copy_buffer.srcBuffer = src_buffer.handle;
		copy_buffer.dstBuffer = dst_buffer->handle;
		copy_buffer.regionCount = 1u;
		copy_buffer.pRegions = &copy_region;
		
		auto& current = command_buffer.get();
		current.copyBuffer2KHR(copy_buffer);
	}

	m_pDevice->endCommandBuffer(command_buffer);

	m_pDevice->submitCommandBuffer(command_buffer);

	// TODO: submit command buffer. But with pushing semaphores for synchronize with main command buffer
	destroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

	// destroy staging buffers
	for (auto& staging_buffer : staging_buffers)
		m_pDevice->destroyBuffer(staging_buffer);

	m_textureUploadInfos.clear();
	m_bufferUploadInfos.clear();
}

void CVkRenderContext::processResourceUtilization()
{
	while (!m_destroyResource.empty())
	{
		auto destroy_resource = m_destroyResource.front();

		if (destroy_resource.mask & EResourceFlagBits::eTexture)
		{
			auto& texture = m_pResources->get<FVkTexture>(destroy_resource.id);
			destroyTexture(*texture);
			m_pResources->remove<FVkTexture>(destroy_resource.id);
		}
		else if (destroy_resource.mask & EResourceFlagBits::eShader)
		{
			auto& shader = m_pResources->get<FVkShader>(destroy_resource.id);
			destroyShader(*shader);
			m_pResources->remove<FVkShader>(destroy_resource.id);
		}
		else if (destroy_resource.mask & EResourceFlagBits::eBuffer)
		{
			auto& buffer = m_pResources->get<FVkBuffer>(destroy_resource.id);
			m_pDevice->destroyBuffer(*buffer);
			m_pResources->remove<FVkBuffer>(destroy_resource.id);
		}

		m_destroyResource.pop();
	}
}

FRenderApiErrorInfo CVkRenderContext::createTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture)
{
	texture.initialData.base = texture_ci.base;
	
	FVkImageCreateInfo image_ci{};
	FVkImageViewCreateInfo image_view_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	texture.name = image_ci.name = texture_ci.name;
#endif

	if (texture_ci.base.depth > 1 || (texture_ci.base.typeFlags & ETextureCreateFlagBits::e3D))
		image_ci.ci.imageType = vk::ImageType::e3D;
	else
	{
		if (texture_ci.base.typeFlags & ETextureCreateFlagBits::e1D)
			image_ci.ci.imageType = vk::ImageType::e1D;
		else if (texture_ci.base.typeFlags & ETextureCreateFlagBits::e2D)
			image_ci.ci.imageType = vk::ImageType::e2D;
		else if (texture_ci.base.typeFlags & ETextureCreateFlagBits::e3D)
			image_ci.ci.imageType = vk::ImageType::e3D;
		else
			image_ci.ci.imageType = vk::ImageType::e2D;
	}

	image_ci.ci.extent = vk::Extent3D{ texture_ci.base.width, texture_ci.base.height, texture_ci.base.depth };
	image_ci.ci.mipLevels = texture_ci.base.mipLevels;

	image_ci.ci.arrayLayers = texture_ci.base.arrayLayers;
	if ((texture_ci.base.typeFlags & ETextureCreateFlagBits::eArray) && (texture_ci.base.typeFlags & ETextureCreateFlagBits::eCubemap))
	{
		image_ci.ci.arrayLayers *= 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	}
	else if (texture_ci.base.typeFlags & ETextureCreateFlagBits::eArray)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if (texture_ci.base.typeFlags & ETextureCreateFlagBits::eCubemap)
	{
		image_ci.ci.arrayLayers = 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	}
	else if (texture_ci.base.typeFlags & ETextureCreateFlagBits::e3D)
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
	image_ci.ci.flags = (texture_ci.base.typeFlags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(m_pDevice->createImage(image_ci, texture.image));

	auto image_aspect = get_texture_aspect_flags(texture);
	if (image_aspect & vk::ImageAspectFlagBits::eStencil)
		image_aspect = vk::ImageAspectFlagBits::eDepth;

	image_view_ci.ci.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	image_view_ci.ci.subresourceRange.aspectMask = image_aspect;
	image_view_ci.ci.subresourceRange.baseArrayLayer = 0u;
	image_view_ci.ci.subresourceRange.layerCount = texture.totalLayers;
	image_view_ci.ci.image = texture.image.handle;

	texture.imageViews.resize(texture_ci.base.mipLevels);
	for (uint32_t mip = 0u; mip < texture_ci.base.mipLevels; ++mip)
	{
		image_view_ci.ci.subresourceRange.baseMipLevel = mip;
		image_view_ci.ci.subresourceRange.levelCount = mip == 0u ? texture_ci.base.mipLevels : 1u;
#ifndef BUILD_WITHOUT_DEBUGGIND
		image_view_ci.name = std::format("{} mip #{}", texture_ci.name, mip);
#endif
		RETURN_WHEN_FAULT(m_pDevice->createImageView(image_view_ci, texture.imageViews.at(mip)))
	}

	FVkSamplerCreateInfo sampler_ci{};
	sampler_ci.addressMode = texture_ci.addressMode;
	sampler_ci.enableAnisotropy = texture_ci.enableAnisotropy;
	sampler_ci.enableCompareOp = is_depth_format(image_ci.ci.format);
	sampler_ci.minFilter = sampler_ci.magFilter = texture_ci.filter;
	sampler_ci.mipLevels = texture_ci.base.mipLevels;
#ifndef BUILD_WITHOUT_DEBUGGIND
	sampler_ci.name = texture_ci.name;
#endif

	FRenderApiErrorInfo res = make_api_error(vk::ObjectType::eSampler, vk::Result::eSuccess);
	if(texture_ci.createSampler)
		res = m_pDevice->createSampler(sampler_ci, texture.sampler);

	texture.isAlive = true;

	return res;
}

void CVkRenderContext::destroyTexture(FVkTexture& texture)
{
	m_pDevice->destroySampler(texture.sampler);
	
	for (auto& image_view : texture.imageViews)
		m_pDevice->destroyImageView(image_view);
	texture.imageViews.clear();

	m_pDevice->destroyImage(texture.image);

	texture.isAlive = false;
}

FRenderApiErrorInfo CVkRenderContext::createShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader)
{
	auto& reflection = m_pResources->get<FVkShaderReflectionData>(shader.reflection);

	// TODO: generate better object names. Current names can overlap

	// Remember maximum shader usage count
	shader.maximumDescriptorSlots = m_pDevice->m_swapchain.framesInFlight * resources_info.usageCount;

	// Create descriptor set layouts
	for (const auto& [set, layout_bindings] : reflection->descriptorSetLayoutBindings)
	{
		FVkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
		descriptor_set_layout_ci.name = std::format("{}: set #{}", resources_info.name, set);
#endif
		descriptor_set_layout_ci.layoutBindings = layout_bindings; // TODO: may be send pointer to vector here?

		auto& descriptor_set_Layout = shader.descriptorSetLayouts[set];
		RETURN_WHEN_FAULT(m_pDevice->createDescriptorSetLayout(descriptor_set_layout_ci, descriptor_set_Layout));
	}

	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = resources_info.name;
#endif
	pool_ci.poolSizes = reflection->descriptorPoolSizes;
	pool_ci.usageCount = resources_info.usageCount * m_pDevice->m_swapchain.framesInFlight;
	RETURN_WHEN_FAULT(m_pDevice->createDescriptorPool(pool_ci, shader.descriptorPool));

	// Create descrioptor sets for each shader usage
	for (const auto& [set, layout] : shader.descriptorSetLayouts)
	{
		FVkDescriptorSetCreateInfo descriptor_set_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
		descriptor_set_ci.name = std::format("{}: set #{}", resources_info.name, set);
#endif
		descriptor_set_ci.pool = shader.descriptorPool.handle;
		descriptor_set_ci.layout = layout;
		descriptor_set_ci.descriptorCount = shader.maximumDescriptorSlots;

		auto& descriptor_set = shader.descriptorSets[set];
		RETURN_WHEN_FAULT(m_pDevice->createDescriptorSet(descriptor_set_ci, descriptor_set));
	}

	// Minimal buffer offset
	auto& physical = m_pDevice->getPhysicalDevice();
	auto minOffsetAllignment = std::lcm(physical.properties.limits.minUniformBufferOffsetAlignment, physical.properties.limits.nonCoherentAtomSize);

	// Create uniform buffers
	for (const auto& [id, uniform_block] : reflection->uniformBlocks)
	{
		// Storage buffers is binding from outside
		if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
			continue;

		FVkBufferCreateInfo buffer_ci{};
		buffer_ci.instanceCount = shader.maximumDescriptorSlots;
		buffer_ci.instanceSize = uniform_block.size;
		buffer_ci.minOffsetAlignment = minOffsetAllignment;
		buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
#ifndef BUILD_WITHOUT_DEBUGGIND
		auto* debug_name = debug_mapping::find(id);
		if (debug_name)
			buffer_ci.name = std::format("{}:{}", resources_info.name, debug_name);
		else
			buffer_ci.name = std::format("{}:{:x}", resources_info.name, id);
#endif

		if (uniform_block.descriptorType == vk::DescriptorType::eUniformBuffer)
			buffer_ci.usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		else if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
			buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer;

		auto buffer = std::make_unique<FVkBuffer>();
		RETURN_WHEN_FAULT(m_pDevice->createBuffer(buffer_ci, *buffer));


		auto buffer_name = create_object_unique_name("buffer:{:x}:{}", buffer->handle, id);
		auto buffer_id = m_pResources->add<FVkBuffer>(buffer_name.c_str(), std::move(buffer));
		shader.uniformBuffers.emplace(id, buffer_id);
	}

	// Create pipeline layout
	FVkPipelineLayoutCreateInfo layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	layout_ci.name = resources_info.name;
#endif

	// NOTE: adding global descriptor set layout
	layout_ci.descriptorSetLayouts.emplace_back(m_globalDescriptor.deecriptorSetLayout);

	// Send descriptor set layouts to create info
	for (size_t idx = 1ull; idx < shader.descriptorSetLayouts.size() + 1ull; ++idx)
	{
		if(shader.descriptorSetLayouts.count(idx))
			layout_ci.descriptorSetLayouts.emplace_back(shader.descriptorSetLayouts[idx]);
	}
	//for (const auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
	//	layout_ci.descriptorSetLayouts.emplace_back(descriptor_set_layout);

	// Prepare push blocks info
	uint32_t current_offset{ 0u };
	for (const auto& [name, block] : reflection->pushBlocks)
	{
		auto& push_constant_range = layout_ci.pushConstantRanges.emplace_back();
		push_constant_range.stageFlags = block.stageFlags;
		push_constant_range.offset = current_offset;
		push_constant_range.size = static_cast<uint32_t>(block.size);
		current_offset += push_constant_range.size;
	}

	return m_pDevice->createPipelineLayout(layout_ci, shader.layout);
}

// Destroy shader object
void CVkRenderContext::destroyShader(FVkShader& shader)
{
	for (auto& pipeline : shader.handles)
		m_pDevice->destroyPipeline(pipeline);

	for (auto& shader_module : shader.shaderModules)
		m_pDevice->destroyShaderModule(shader_module);

	for (auto& [set, descriptor_set] : shader.descriptorSets)
		m_pDevice->destroyDescriptorSet(shader.descriptorPool, descriptor_set);

	for (auto& [id, buffer_id] : shader.uniformBuffers)
	{
		auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
		m_pDevice->destroyBuffer(*buffer);
		m_pResources->remove<FVkBuffer>(buffer_id);
	}

	m_pDevice->destroyDescriptorPool(shader.descriptorPool);

	m_pDevice->destroyPipelineLayout(shader.layout);

	for (auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		m_pDevice->destroyDescriptorSetLayout(descriptor_set_layout);

	shader.isAlive = false;
}

void CVkRenderContext::bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader)
{
	std::vector<vk::DescriptorSet> bindDescriptorSets{};

	// NOTE: bind global descriptor set
	bindDescriptorSets.emplace_back(m_globalDescriptor.descriptorSet.handles.at(m_pDevice->m_swapchain.currentFrame));

	// NOTE: shader.descriptorUsage contains next index of descriptor set to use. 
	// Here i collecting all shader descriptor sets, that was changed in past
	for (auto& [set, usage_index] : shader.descriptorUsage)
	{
		auto actual_descriptor_usage = usage_index == 0u ? shader.maximumDescriptorSlots - 1u : usage_index - 1u;
		auto found_descriptor_sets = shader.descriptorSets.find(set);
		if (found_descriptor_sets != shader.descriptorSets.end())
		{
			auto& descriptor_set = found_descriptor_sets->second;
			auto& target_descriptor_set = descriptor_set.handles.at(actual_descriptor_usage);
			bindDescriptorSets.emplace_back(target_descriptor_set);
		}
	}

	// TODO: determine the lowest descriptor index and start binding from lowest one (for example from 1)
	if (!bindDescriptorSets.empty())
	{
		auto& current_command_buffer = command_buffer.get();
		current_command_buffer.bindDescriptorSets(shader.bindPoint, shader.layout, 0, static_cast<uint32_t>(bindDescriptorSets.size()), bindDescriptorSets.data(), 0u, nullptr);
	}
}

void CVkRenderContext::processImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info)
{
	FVkTranslationInfo translation_info{};
	translation_info.imageTranslationInfos.emplace_back(image_translation_info);
	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::textureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info)
{
	auto image_layout_translation = get_texture_layout_translation_info(texture, translation_info);
	processImageLayoutTranslation(command_buffer, image_layout_translation);
}

void CVkRenderContext::copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const std::vector<vk::BufferImageCopy2KHR>& copy_regions)
{
	auto& current_buffer = command_buffer.get();

	vk::CopyBufferToImageInfo2KHR copy_info{};
	copy_info.srcBuffer = buffer.handle;
	copy_info.dstImage = texture.image.handle;
	copy_info.dstImageLayout = texture.layout;
	copy_info.regionCount = static_cast<uint32_t>(copy_regions.size());
	copy_info.pRegions = copy_regions.data();

	current_buffer.copyBufferToImage2KHR(&copy_info);
}
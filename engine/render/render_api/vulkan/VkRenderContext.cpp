#include "VkRenderContext.h"

#include "VkShaderReflection.h"
#include "VkCommon.h"

#include <logger/logger.h>

#include <numeric>
#include <unordered_set>

using namespace engine;
using namespace engine::render;
using namespace engine::render::vulkan;

static const u32 k_max_bindless_buffers{ 256u };

static const u32 k_max_bindless_resources{ 16384u };
static const u64 k_max_texture_update_per_frame{ 4096ll };
static const u64 k_max_buffer_update_per_frame{ 4096ull };

vk::Format lookup_vk_format(EApiDataFormat format)
{
	static const utl::hash_map<EApiDataFormat, vk::Format> lookup_table{
		{ EApiDataFormat::eD32SFLOAT_8UINT, vk::Format::eD32SfloatS8Uint },
		{ EApiDataFormat::eD24UNORM_8UINT, vk::Format::eD24UnormS8Uint },
		{ EApiDataFormat::eD16UNORM_8UINT, vk::Format::eD16UnormS8Uint },
		{ EApiDataFormat::eD32SFLOAT, vk::Format::eD32Sfloat },
		{ EApiDataFormat::eD16UNORM, vk::Format::eD16Unorm },

		{ EApiDataFormat::eBC1RGB_UNORM, vk::Format::eBc1RgbUnormBlock },
		{ EApiDataFormat::eBC1RGB_SRGB, vk::Format::eBc1RgbSrgbBlock },
		{ EApiDataFormat::eBC1RGBA_UNORM, vk::Format::eBc1RgbaUnormBlock },
		{ EApiDataFormat::eBC1RGBA_SRGB, vk::Format::eBc1RgbaSrgbBlock },
		{ EApiDataFormat::eBC2_UNORM, vk::Format::eBc2UnormBlock },
		{ EApiDataFormat::eBC2_SRGB, vk::Format::eBc2SrgbBlock },
		{ EApiDataFormat::eBC_UNORM, vk::Format::eBc3UnormBlock },
		{ EApiDataFormat::eBC3_SRGB, vk::Format::eBc3SrgbBlock },
		{ EApiDataFormat::eBC4_UNORM, vk::Format::eBc4UnormBlock },
		{ EApiDataFormat::eBC4_SNORM, vk::Format::eBc4SnormBlock },
		{ EApiDataFormat::eBC5_UNORM, vk::Format::eBc5UnormBlock },
		{ EApiDataFormat::eBC5_SNORM, vk::Format::eBc5SnormBlock },
		{ EApiDataFormat::eBC6H_UFLOAT, vk::Format::eBc6HUfloatBlock },
		{ EApiDataFormat::eBC6H_SFLOAT, vk::Format::eBc6HSfloatBlock },
		{ EApiDataFormat::eBC7_UNORM, vk::Format::eBc7UnormBlock },
		{ EApiDataFormat::eBC7_SRGB, vk::Format::eBc7SrgbBlock },

		{ EApiDataFormat::eRGBA8_SRGB, vk::Format::eR8G8B8A8Srgb },

		{ EApiDataFormat::eR8_UNORM, vk::Format::eR8Unorm },
		{ EApiDataFormat::eRG8_UNORM, vk::Format::eR8G8Unorm },
		{ EApiDataFormat::eRGB8_UNORM, vk::Format::eR8G8B8Unorm },
		{ EApiDataFormat::eRGBA8_UNORM, vk::Format::eR8G8B8A8Unorm },

		{ EApiDataFormat::eR16_UNORM, vk::Format::eR16Unorm },
		{ EApiDataFormat::eRG16_UNORM, vk::Format::eR16G16Unorm },
		{ EApiDataFormat::eRGB16_UNORM, vk::Format::eR16G16B16Unorm },
		{ EApiDataFormat::eRGBA16_UNORM, vk::Format::eR16G16B16A16Unorm },

		{ EApiDataFormat::eR32_UINT, vk::Format::eR32Uint },
		{ EApiDataFormat::eRG32_UINT, vk::Format::eR32G32Uint },
		{ EApiDataFormat::eRGB32_UINT, vk::Format::eR32G32B32Uint },
		{ EApiDataFormat::eRGBA32_UINT, vk::Format::eR32G32B32A32Uint },

		{ EApiDataFormat::eR16_SFLOAT, vk::Format::eR16Sfloat },
		{ EApiDataFormat::eRG16_SFLOAT, vk::Format::eR16G16Sfloat },
		{ EApiDataFormat::eRGB16_SFLOAT, vk::Format::eR16G16B16Sfloat },
		{ EApiDataFormat::eRGBA16_SFLOAT, vk::Format::eR16G16B16A16Sfloat },

		{ EApiDataFormat::eR32_SFLOAT, vk::Format::eR32Sfloat },
		{ EApiDataFormat::eRG32_SFLOAT, vk::Format::eR32G32Sfloat },
		{ EApiDataFormat::eRGB32_SFLOAT, vk::Format::eR32G32B32Sfloat },
		{ EApiDataFormat::eRGBA32_SFLOAT, vk::Format::eR32G32B32A32Sfloat },

		// Special formats
		{ EApiDataFormat::eR11G11B10_UFLOAT, vk::Format::eB10G11R11UfloatPack32 },
		{ EApiDataFormat::eR10G10B10A2_UNORM, vk::Format::eA2B10G10R10UnormPack32 }
	};

	if (auto found = lookup_table.find(format); found != lookup_table.end())
		return found->second;

	return vk::Format::eUndefined;
}

u64 calculate_sampler_hash(const FVkSamplerCreateInfo& sampler_ci)
{
	static std::hash<u32> hasher;
	u64 hash{ 0ull };

	hash ^= hasher(utl::enum_cast(sampler_ci.magFilter));
	hash ^= hasher(utl::enum_cast(sampler_ci.minFilter));
	hash ^= hasher(utl::enum_cast(sampler_ci.addressMode));
	hash ^= hasher(utl::cast<u32>(sampler_ci.enableAnisotropy));
	hash ^= hasher(utl::cast<u32>(sampler_ci.enableCompareOp));
	hash ^= hasher(sampler_ci.mipLevels);

	return hash;
}

template<class _Ty, class ..._Args>
std::string create_object_unique_name(const char* fmt, const _Ty& handle, _Args&&... args)
{
	auto object_handle = reinterpret_cast<u64>(static_cast<_Ty::CType>(handle));
	return std::vformat(fmt, std::make_format_args(object_handle, std::forward<_Args>(args)...));
}

CVkRenderContext::~CVkRenderContext()
{
	auto& logical_device = m_pDevice->GetLogicalDevice();
	logical_device.handle.waitIdle();

	// Destroy thread contexts
	for (auto& [thread_id, context] : m_perThreadContext)
		destroyThreadContext(context);

	// Destroy all textures
	auto& textures = m_pResources->data<FVkTexture>();
	for (resource_handle_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
	{
		auto& texture = textures.at(texture_id);
		if (!texture->isAlive)
			continue;

		DestroyTexture(*texture);
	}

	// Destroy all shaders
	auto& shaders = m_pResources->data<FVkShader>();
	for (resource_handle_t shader_id = 0ull; shader_id < shaders.size(); ++shader_id)
	{
		auto& shader = shaders.at(shader_id);
		if (!shader->isAlive)
			continue;

		DestroyShader(*shader);
	}

	// Destroy all buffers
	auto& buffers = m_pResources->data<FVkBuffer>();
	for (resource_handle_t buffer_id = 0ull; buffer_id < buffers.size(); ++buffer_id)
	{
		auto& buffer = buffers.at(buffer_id);
		if (!buffer->isAlive)
			continue;

		m_pDevice->DestroyBuffer(*buffer);
	}

	m_pDevice->DestroySampler(m_defaultSampler);

	for(auto& [uid, sampler] : m_samplerCache)
		m_pDevice->DestroySampler(sampler);
}

void CVkRenderContext::Create(const FRenderContextCreateInfo& context_ci)
{
	m_pDevice = std::make_unique<CVkRenderDevice>();
	m_pDevice->Create(context_ci);

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

	CreateCommandBuffer(command_buffer_ci);

	auto res = m_pDevice->CreateTimestampQuery(256, m_timestampQuery);

	auto& per_thread = m_perThreadContext[std::this_thread::get_id()];
	auto& per_queue = per_thread.perQueue[EQueueTypeFlagBits::eGraphics];
	m_pDevice->InitProfiler(per_queue.commandPool);

	updateBackbuffer();

	// Create default sampler
	FVkSamplerCreateInfo sampler_ci{};
	sampler_ci.addressMode = vk::SamplerAddressMode::eClampToEdge;
	sampler_ci.enableAnisotropy = false;
	sampler_ci.enableCompareOp = false;
	sampler_ci.minFilter = sampler_ci.magFilter = vk::Filter::eLinear;
	sampler_ci.mipLevels = 1u;
#ifndef BUILD_WITHOUT_DEBUGGIND
	sampler_ci.name = "default";
#endif

	m_pDevice->CreateSampler(sampler_ci, m_defaultSampler);
}

void CVkRenderContext::CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci)
{
	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
	pool_ci.poolSizes = { { vk::DescriptorType::eCombinedImageSampler, k_max_bindless_resources }, {vk::DescriptorType::eUniformBuffer, 3u} };
	pool_ci.usageCount = 1u * m_pDevice->m_swapchain.framesInFlight;
	pool_ci.flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = "global:textures";
#endif

	m_pDevice->CreateDescriptorPool(pool_ci, m_globalDescriptor.descriptorPool);

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

	// Uniform buffer with post processing data
	auto& global_postprocess_binding = set_layout_ci.layoutBindings.emplace_back();
	global_postprocess_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
	global_postprocess_binding.descriptorCount = 1u;
	global_postprocess_binding.binding = 2u;
	global_postprocess_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;

	// Bindles textures
	auto& global_textures_binding = set_layout_ci.layoutBindings.emplace_back();
	global_textures_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	global_textures_binding.descriptorCount = k_max_bindless_resources;
	global_textures_binding.binding = 3u;
	global_textures_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
	global_textures_binding.pImmutableSamplers = nullptr;

	// Descriptor binding extension
	utl::vector<vk::DescriptorBindingFlags> bindingFlags{};
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

	vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCIEXT{};
	descriptorSetLayoutBindingFlagsCIEXT.bindingCount = utl::cast<u32>(bindingFlags.size());
	descriptorSetLayoutBindingFlagsCIEXT.pBindingFlags = bindingFlags.data();

	set_layout_ci.next = &descriptorSetLayoutBindingFlagsCIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_layout_ci.name = "global:textures";
#endif

	m_pDevice->CreateDescriptorSetLayout(set_layout_ci, m_globalDescriptor.deecriptorSetLayout);

	// Create descriptor set
	FVkDescriptorSetCreateInfo set_ci{};
	set_ci.descriptorCount = m_pDevice->m_swapchain.framesInFlight;
	set_ci.layout = m_globalDescriptor.deecriptorSetLayout;
	set_ci.pool = m_globalDescriptor.descriptorPool.handle;

	// TODO: Potential problems here in future. I think that max_sets array is incorrect
	utl::vector<u32> max_sets(m_pDevice->m_swapchain.framesInFlight, k_max_bindless_resources - 1);
	vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT descriptor_set_variable_descriptor_count_AIEXT{};
	descriptor_set_variable_descriptor_count_AIEXT.descriptorSetCount = m_pDevice->m_swapchain.framesInFlight;
	descriptor_set_variable_descriptor_count_AIEXT.pDescriptorCounts = max_sets.data();
	set_ci.next = &descriptor_set_variable_descriptor_count_AIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_ci.name = "global:textures";
#endif

	m_pDevice->CreateDescriptorSet(set_ci, m_globalDescriptor.descriptorSet);

	// Create uniform buffers
	auto& physical = m_pDevice->GetPhysicalDevice();

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
		m_pDevice->CreateBuffer(buffer_ci, *buffer);

		auto buffer_name = create_object_unique_name("global:{:x}:{:x}", buffer->handle, uniform_name);
		auto buffer_id = m_pResources->add<FVkBuffer>(buffer_name.c_str(), std::move(buffer));
		m_globalDescriptor.uniformBuffers.emplace(uniform_name, buffer_id);
	}
}

void CVkRenderContext::UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view)
{
	auto& dest_set = m_globalDescriptor.descriptorSet.handles.at(m_pDevice->m_swapchain.currentFrame);

	std::array<vk::DescriptorImageInfo, k_max_texture_update_per_frame> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, k_max_buffer_update_per_frame> buffer_descriptors{};
	utl::vector<vk::WriteDescriptorSet> write_descriptor_sets{};

	auto& textures = m_pResources->data<FVkTexture>();

	// Bind and update global uniform
	u64 buffer_count{ 0ull };
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
		m_pDevice->MapBufferMemory(*buffer, &mapped_memory);
		std::memcpy((char*)mapped_memory + buffer_descriptor.offset, uniform_data.data, uniform_data.size);
		m_pDevice->UnmapBufferMemory(*buffer);
		buffer_count++;
	}

	// Process bindless textures
	{
		auto& texture_queue = m_textureToUpdate[m_pDevice->m_swapchain.currentFrame];

		u64 texture_count{ 0ull };
		while (!texture_queue.empty() && texture_count < k_max_texture_update_per_frame)
		{
			auto texture_id = texture_queue.front();
			auto& texture = textures.at(texture_id);

			auto& start_element_index = m_textureIndexRemap.at(texture_id);
			bool is_storage_image = utl::cast<bool>(texture->info.typeFlags & ETextureCreateFlagBits::eStorage);

			auto& mip_level = texture->getSubresource();
			auto& image_descriptor = image_descriptors.at(texture_count);
			image_descriptor.imageView = mip_level.imageView;
			image_descriptor.sampler = texture->sampler != ~0ull ? m_samplerCache[texture->sampler] : m_defaultSampler;
			image_descriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (texture->info.typeFlags & ETextureCreateFlagBits::eSamplable)
			{
				auto& write_descriptor_set = write_descriptor_sets.emplace_back();
				write_descriptor_set.dstArrayElement = start_element_index;
				write_descriptor_set.descriptorType = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_set.dstSet = dest_set;
				write_descriptor_set.dstBinding = 3u;
				write_descriptor_set.descriptorCount = 1u;
				write_descriptor_set.pImageInfo = &image_descriptor;
			}

			texture_count++;

			texture_queue.pop();
		}
	}

	// Update descriptor sets if needed
	if (!write_descriptor_sets.empty())
	{
		auto& logical = m_pDevice->GetLogicalDevice();
		logical.handle.updateDescriptorSets(utl::cast<u32>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
	}
}

const utl::vector<uint32_t>& CVkRenderContext::GetGlobalTextureRemap()
{
	return m_textureIndexRemap;
}

u64 CVkRenderContext::CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info)
{
	// Create texture handle
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.format = lookup_vk_format(texture_ci.format);
	vk_texture_ci.typeFlags = texture_ci.typeFlags;
	vk_texture_ci.size = texture_ci.size;
	vk_texture_ci.mipLevels = texture_ci.mipLevels;
	vk_texture_ci.arrayLayers = texture_ci.arrayLayers;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eRepeat;
	vk_texture_ci.enableAnisotropy = true;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	auto texture = std::make_unique<FVkTexture>();
	CreateTexture(vk_texture_ci, *texture);

	auto& upload_data = m_textureUploadInfos.emplace_back(std::move(upload_info));
	upload_data.textureId = m_pResources->add<FVkTexture>(name.c_str(), std::move(texture));

	return upload_data.textureId;
}

u64 CVkRenderContext::GetTextureId(const std::string& name)
{
	return m_pResources->getId<FVkTexture>(name.c_str());
}

u64 CVkRenderContext::CreateRenderTarget(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.format = lookup_vk_format(texture_ci.format);
	vk_texture_ci.typeFlags = texture_ci.typeFlags;
	vk_texture_ci.size = texture_ci.size;
	vk_texture_ci.mipLevels = texture_ci.mipLevels;
	vk_texture_ci.arrayLayers = texture_ci.arrayLayers;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eClampToEdge;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	auto texture = std::make_unique<FVkTexture>();
	texture->isRenderTarget = true;

	CreateTexture(vk_texture_ci, *texture);

	return m_pResources->add<FVkTexture>(name.c_str(), std::move(texture));
}

FTextureView CVkRenderContext::GetTextureView(u64 texture_id)
{
	auto& texture = m_pResources->get<FVkTexture>(texture_id);

	FTextureView tex_view{};
	tex_view.width = texture->info.size.width;
	tex_view.height = texture->info.size.height;
	tex_view.depth = texture->info.size.depth;
	tex_view.arrayLayers = texture->info.arrayLayers;
	tex_view.mipLevels = texture->info.mipLevels;

	return tex_view;
}

void CVkRenderContext::DestroyTexture(u64 resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eTexture });
}


u64 CVkRenderContext::CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	CVkShaderReflectionBuilder reflection_builder{};

	FVkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.primitiveTopology = get_primitive_topology(shader_ci.primitiveTopology);
	pipeline_ci.cullMode = get_cull_mode_flags(shader_ci.cullMode);
	pipeline_ci.frontFace = get_front_face(shader_ci.frontFace);
	pipeline_ci.depthCompareOp = get_compare_op(shader_ci.depthCompareOp);
#ifndef BUILD_WITHOUT_DEBUGGIND
	pipeline_ci.name = name;
#endif

	// Get color attachment formats
	auto& textures = m_pResources->data<FVkTexture>();
	for (auto& attachment : shader_ci.colorAttachments)
	{
		FVkTexture* texture{ nullptr };
		if (attachment.backbuffer)
			texture = &m_backbuffer[m_pDevice->m_swapchain.currentFrame];
		else
			texture = textures[attachment.source].get();

		pipeline_ci.colorAttachmentFormats.emplace_back(texture->info.format);
	}

	// Get depth format
	if(shader_ci.depthAttachment.source != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.depthAttachment.source);
		pipeline_ci.depthAttachmentFormat = texture->info.format;
	}

	// Get stencil format
	if (shader_ci.stencilAttachment.source != invalid_handle)
	{
		auto& texture = textures.at(shader_ci.stencilAttachment.source);
		pipeline_ci.stencilAttachmentFormat = texture->info.format;
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
		description.format = lookup_vk_format(vertex_binding.format);
		description.offset = vertex_binding.offset;
	}

	// Create shader modules and collect reflection data
	u32 shader_index{ 0u };
	for (const auto& [stage, code] : shader_stages)
	{
		auto shader_stage = get_shader_stage(stage);
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", name, vk::to_string(shader_stage), shader_index);
#endif

		auto& shader_module = shader->shaderModules.emplace_back();
		m_pDevice->CreateShaderModule(shader_module_ci, shader_module);

		auto& stage_ci = pipeline_ci.shaderStages.emplace_back();
		stage_ci.stage = shader_stage;
		stage_ci.module = shader_module;
		stage_ci.pName = shaderEntryPoint;

		reflection_builder.AddShaderStage(shader_stage, *code);
		shader_index++;
	}

	auto reflection = std::make_unique<FVkShaderReflectionData>(reflection_builder.Build());

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
	m_pDevice->CreatePipeline(pipeline_ci, pipeline);
	
	shader->isAlive = true;

	auto shader_unique_name = create_object_unique_name("shader:{:x}:{}", shader->handles.back(), vk::to_string(shader->bindPoint));
	return m_pResources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader));
}

u64 CVkRenderContext::CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	CVkShaderReflectionBuilder reflection_builder{};
	utl::vector<vk::PipelineShaderStageCreateInfo> pipeline_stages{};

	// Create shader modules and collect reflection data
	for (u64 idx = 0ull; idx < shader_stages.size(); ++idx)
	{
		auto& code = shader_stages.at(idx);
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code.code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", name, vk::to_string(vk::ShaderStageFlagBits::eCompute), idx);
#endif

		auto& shader_module = shader->shaderModules.emplace_back();
		m_pDevice->CreateShaderModule(shader_module_ci, shader_module);

		auto& stage = pipeline_stages.emplace_back();
		stage.stage = vk::ShaderStageFlagBits::eCompute;
		stage.module = shader_module;
		stage.pName = shaderEntryPoint;

		reflection_builder.AddShaderStage(vk::ShaderStageFlagBits::eCompute, *code.code);
	}

	auto reflection = std::make_unique<FVkShaderReflectionData>(reflection_builder.Build());

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

	for (u64 idx = 0ull; idx < pipeline_stages.size(); ++idx)
	{
		auto& stage = pipeline_stages.at(idx);
		pipeline_ci.shaderStage = stage;
#ifndef BUILD_WITHOUT_DEBUGGIND
		pipeline_ci.name = std::format("{} #{}", name, idx);
#endif

		auto& pipeline = shader->handles.emplace_back();
		m_pDevice->CreatePipeline(pipeline_ci, pipeline);
	}

	shader->bindPoint = vk::PipelineBindPoint::eCompute;

	std::stringstream ss{};
	ss << name;
	for (const auto& handle : shader->handles)
		ss << ":" << std::hex << reinterpret_cast<u64>(utl::cast<vk::Pipeline::CType>(handle));

	auto shader_unique_name = ss.str();
	return m_pResources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader));
}

u64 CVkRenderContext::CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, const  utl::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();




	shader->bindPoint = vk::PipelineBindPoint::eRayTracingKHR;

	std::stringstream ss{};
	ss << name;
	for (const auto& handle : shader->handles)
		ss << ":" << std::hex << reinterpret_cast<u64>(utl::cast<vk::Pipeline::CType>(handle));

	auto shader_unique_name = ss.str();
	return m_pResources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader));
}

FShaderReflectionData CVkRenderContext::GetShaderReflection(const u64& shader_view)
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
void CVkRenderContext::UpdateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags)
{
	auto& thread_context = m_perThreadContext[std::this_thread::get_id()];

	auto& shader = m_pResources->get<FVkShader>(shader_resources.shaderId);
	auto& shader_reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);

	auto& textures = m_pResources->data<FVkTexture>();
	auto& buffers = m_pResources->data<FVkBuffer>();

	std::unordered_set<u32> updated_descriptor_sets{};
	std::array<vk::WriteDescriptorSet, 64ull> write_descriptor_sets{};
	std::array<vk::DescriptorImageInfo, 64ull> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, 64ull> buffer_descriptors{};

	u64 texture_count{ 0ull };
	u64 buffer_count{ 0ull };
	u64 writes_count{ 0ull };

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
			auto& buffer = buffers.at(buffer_id);
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
			m_pDevice->MapBufferMemory(*buffer, &mapped_memory);
			std::memcpy((char*)mapped_memory + buffer_descriptor.offset, uniform_data.data, uniform_data.size);
			m_pDevice->UnmapBufferMemory(*buffer);
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

			auto& buffer = buffers.at(buffer_id);

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
			auto& texture = textures.at(bind_info.textureId);
			auto& texture_subresource = texture->getSubresource(bind_info.mipLevel);

			auto found_reflection = shader_reflection->uniforms.find(texture_name);
			if (found_reflection == shader_reflection->uniforms.end())
				continue;

			auto& uniform = found_reflection->second;
			updated_descriptor_sets.emplace(uniform.set);

			auto& image_descriptor = image_descriptors.at(texture_count);
			image_descriptor.imageView = texture_subresource.imageView;
			image_descriptor.sampler = texture->sampler != ~0ull ? m_samplerCache[texture->sampler] : m_defaultSampler;
			image_descriptor.imageLayout = texture_subresource.layout;
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
		auto& logical = m_pDevice->GetLogicalDevice();
		logical.handle.updateDescriptorSets(utl::cast<u32>(writes_count), write_descriptor_sets.data(), 0, nullptr);
	}
}

void CVkRenderContext::FlushConstantRanges(u64 shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type)
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

void CVkRenderContext::BindShader(u64 shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& shader = m_pResources->get<FVkShader>(shader_id);

	// TODO: make possible to select what pipeline i have to bind now
	current.bindPipeline(shader->bindPoint, shader->handles.back());
}

void CVkRenderContext::BindShaderResources(u64 shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& shader = m_pResources->get<FVkShader>(shader_id);

	bindShaderDescriptors(command_buffer, *shader);
}

void CVkRenderContext::DestroyShader(u64 resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eShader });
}

u64 CVkRenderContext::CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto vk_buffer_ci = makeBufferCreateInfo(name, buffer_ci);

	auto buffer = std::make_unique<FVkBuffer>();
	m_pDevice->CreateBuffer(vk_buffer_ci, *buffer);

	auto buffer_unique_name = create_object_unique_name("buffer:{:x}:{}", buffer->handle, name);
	return m_pResources->add<FVkBuffer>(buffer_unique_name.c_str(), std::move(buffer));
}

void CVkRenderContext::UpdateBuffer(const std::string& name, u64 buffer_id, const FBufferCreateInfo& buffer_ci)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->DestroyBuffer(*buffer);

	auto vk_buffer_ci = makeBufferCreateInfo(name, buffer_ci);
	m_pDevice->CreateBuffer(vk_buffer_ci, *buffer);
}

void CVkRenderContext::UploadDataToBuffer(const FBufferUploadInfo& upload_info)
{
	m_bufferUploadInfos.emplace_back(upload_info);
}

FBufferView CVkRenderContext::GetBufferView(u64 buffer_id)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	return { buffer->size, buffer->instanceCount, buffer->address };
}

void CVkRenderContext::BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	utl::vector<vk::Buffer> buffers{};
	for (const auto& buffer_id : binding_info.buffers)
		buffers.emplace_back(m_pResources->get<FVkBuffer>(buffer_id)->handle);

	current.bindVertexBuffers(binding_info.firstBinding, buffers, binding_info.offsets);
}

void CVkRenderContext::BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
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

void CVkRenderContext::MapBufferMemory(u64 buffer_id, void*& memory)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->MapBufferMemory(*buffer, &memory);
}

void CVkRenderContext::UnmapBufferMemory(u64 buffer_id)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->UnmapBufferMemory(*buffer);
}

void CVkRenderContext::FlushBuffer(u64 buffer_id, u64 size, u64 offset)
{
	auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
	m_pDevice->flushBufferMemory(*buffer, size, offset);
}

void CVkRenderContext::DestroyBuffer(u64 resource_id)
{
	m_destroyResource.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eBuffer });
}

void CVkRenderContext::UploadBuffers()
{
	processResourceUpload();
}

void CVkRenderContext::PushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures.at(texture_id);

		auto& image_trans_info = translation_info.imageTranslationInfos.emplace_back();
		image_trans_info.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		image_trans_info.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
		image_trans_info.oldLayout = vk::ImageLayout::eUndefined;
		image_trans_info.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
		image_trans_info.newLayout = get_render_target_final_layout(*texture);
		image_trans_info.handle = texture->image.handle;

		image_trans_info.subresourceRange.aspectMask = get_texture_aspect_flags(*texture);
		image_trans_info.subresourceRange.baseArrayLayer = 0;
		image_trans_info.subresourceRange.layerCount = texture->totalLayers;
		image_trans_info.subresourceRange.baseMipLevel = 0;
		image_trans_info.subresourceRange.levelCount = texture->info.mipLevels;
	}

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::PushC2CBarrier(EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	auto& barrier = translation_info.memoryTranslationInfos.emplace_back();
	barrier.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.srcAccessMask = vk::AccessFlagBits2::eShaderWrite;
	barrier.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::PushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<u64>& texture_ids)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& textures = m_pResources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures.at(texture_id);
	
		auto& image_trans_info = translation_info.imageTranslationInfos.emplace_back();
		image_trans_info.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
		image_trans_info.srcAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eMemoryWrite;
		image_trans_info.oldLayout = get_render_target_final_layout(*texture);
		image_trans_info.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
		image_trans_info.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderSampledRead;
		image_trans_info.newLayout = get_render_target_final_layout(*texture);
		image_trans_info.handle = texture->image.handle;
	
		image_trans_info.subresourceRange.aspectMask = get_texture_aspect_flags(*texture);
		image_trans_info.subresourceRange.baseArrayLayer = 0;
		image_trans_info.subresourceRange.layerCount = texture->totalLayers;
		image_trans_info.subresourceRange.baseMipLevel = 0;
		image_trans_info.subresourceRange.levelCount = texture->info.mipLevels;
	}

	//auto& barrier = translation_info.memoryTranslationInfos.emplace_back();
	//barrier.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	//barrier.srcAccessMask = vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
	//barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
	//barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eMemoryRead;

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers)
{
	auto& command_buffer = getLastCommandBuffer(queue);

	auto& textures = m_pResources->data<FVkTexture>();
	auto& buffers = m_pResources->data<FVkBuffer>();

	FVkTranslationInfo translation_info{};

	for (const auto& resource_barrier : barriers)
	{
		ERenderResourceAccessFlagBits src_resource_access{};
		ERenderResourceAccessFlagBits dst_resource_access{};
		unpackDependencyBarrier(resource_barrier.dependencyType, src_resource_access, dst_resource_access);

		if (resource_barrier.barrierType == ERenderResourceBarrierType::eMemory)
		{
			auto& memory_barrier = translation_info.memoryTranslationInfos.emplace_back();

			if (resource_barrier.resource.type & ERenderResourceTypeFlagBits::eAllImage)
			{
				auto& texture = textures.at(resource_barrier.resource.handle);
				auto& texture_subresource = texture->getSubresource(resource_barrier.subresourceIndex);

				memory_barrier.srcStageMask = get_translation_stage_flags(texture_subresource.layout, resource_barrier.srcPassType);
				memory_barrier.srcAccessMask = get_translation_access_flags(texture_subresource.layout, resource_barrier.srcPassType, src_resource_access);
				memory_barrier.dstStageMask = get_translation_stage_flags(texture_subresource.layout, resource_barrier.dstPassType);
				memory_barrier.dstAccessMask = get_translation_access_flags(texture_subresource.layout, resource_barrier.dstPassType, dst_resource_access);
			}
			else
			{
				memory_barrier.srcStageMask = get_translation_stage_flags(resource_barrier.srcPassType);
				memory_barrier.srcAccessMask = get_translation_access_flags(resource_barrier.srcPassType, src_resource_access);
				memory_barrier.dstStageMask = get_translation_stage_flags(resource_barrier.dstPassType);
				memory_barrier.dstAccessMask = get_translation_access_flags(resource_barrier.dstPassType, dst_resource_access);
			}
		}
		else
		{
			if (resource_barrier.resource.type & ERenderResourceTypeFlagBits::eAllImage)
			{
				auto& texture_barrier = translation_info.imageTranslationInfos.emplace_back();
				texture_barrier.subresourceRange.baseArrayLayer = 0u;
				texture_barrier.subresourceRange.baseMipLevel = 0u;

				FVkTexture* texture{ nullptr };
				if (resource_barrier.resource.type & ERenderResourceTypeFlagBits::eBackbuffer)
					texture = &m_backbuffer[m_pDevice->m_swapchain.currentFrame];
				else
					texture = textures[resource_barrier.resource.handle].get();

				auto& texture_subresource = texture->getSubresource(resource_barrier.subresourceIndex);

				vk::ImageLayout old_image_layout{ vk::ImageLayout::eUndefined }, new_image_layout{ vk::ImageLayout::eUndefined };
				vk::ImageAspectFlags aspect_flags{};

				if (resource_barrier.barrierType == ERenderResourceBarrierType::eIntermediate)
					new_image_layout = get_render_target_intermediate_layout(*texture);
				else if (resource_barrier.barrierType == ERenderResourceBarrierType::eFinal)
					new_image_layout = get_render_target_final_layout(*texture);

				old_image_layout = texture_subresource.layout;
				texture_subresource.layout = new_image_layout;
				texture_barrier.handle = texture->image.handle;
				texture_barrier.subresourceRange.aspectMask = get_texture_aspect_flags(*texture);

				// TODO: Fix here required. Sometimes i need to process all layers and levels
				if (resource_barrier.subresourceIndex == ~0u)
				{
					for (auto& mip : texture->mips)
						mip.layout = new_image_layout;
				}

				texture_barrier.subresourceRange.baseMipLevel = texture_subresource.baseMipLevel;
				texture_barrier.subresourceRange.levelCount = texture_subresource.mipLevelCount;
				texture_barrier.subresourceRange.layerCount = texture->totalLayers;

				texture_barrier.srcStageMask = get_translation_stage_flags(old_image_layout, resource_barrier.srcPassType);
				texture_barrier.srcAccessMask = get_translation_access_flags(old_image_layout, resource_barrier.srcPassType, src_resource_access);
				texture_barrier.oldLayout = old_image_layout;
				texture_barrier.dstStageMask = get_translation_stage_flags(new_image_layout, resource_barrier.dstPassType);
				texture_barrier.dstAccessMask = get_translation_access_flags(new_image_layout, resource_barrier.dstPassType, dst_resource_access);
				texture_barrier.newLayout = new_image_layout;
			}
			// For buffers
			else
			{
				auto& buffer = buffers.at(resource_barrier.resource.handle);

				FVkBufferTranslationInfo buffer_translation_info{};
				buffer_translation_info.handle = buffer->handle;
				buffer_translation_info.size = buffer->size;
				buffer_translation_info.srcStageMask = get_translation_stage_flags(resource_barrier.srcPassType);
				buffer_translation_info.srcAccessMask = get_translation_access_flags(resource_barrier.srcPassType, src_resource_access);
				buffer_translation_info.dstStageMask = get_translation_stage_flags(resource_barrier.dstPassType);
				buffer_translation_info.dstAccessMask = get_translation_access_flags(resource_barrier.dstPassType, dst_resource_access);
				translation_info.bufferTranslationInfos.emplace_back(buffer_translation_info);
			}
		}
	}

	processMemoryBarriers(command_buffer, translation_info);
}

void CVkRenderContext::BeginTimestampZone(EQueueTypeFlagBits queue)
{
	//assert(m_zoneEntries[queue] == 0 && "Previous zone was not ended.");
	auto& command_buffer = getLastCommandBuffer(queue);
	m_pDevice->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eTopOfPipe, m_timestampQuery);
}

void CVkRenderContext::EndTimestampZone(EQueueTypeFlagBits queue)
{
	auto& command_buffer = getLastCommandBuffer(queue);
	m_pDevice->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eBottomOfPipe, m_timestampQuery);
}

void CVkRenderContext::BeginRendering(const FBeginRendering2Info& rendering_info)
{
	auto& command_buffer = getLastCommandBuffer(rendering_info.queue);

	auto& textures = m_pResources->data<FVkTexture>();

	utl::hash_map<u64, vk::ClearValue> clear_values{};
	for (const auto& [handle, clear_value] : rendering_info.clearValues)
	{
		auto& new_value = clear_values[handle];

		if (handle != INVALID_HANDLE)
		{
			auto& texture = textures[handle];

			auto aspect = get_texture_aspect_flags(*texture);
			if(aspect & vk::ImageAspectFlagBits::eColor)
				new_value.color = vk::ClearColorValue(clear_value.color.x, clear_value.color.y, clear_value.color.z, clear_value.color.w);
			else
				new_value.depthStencil = vk::ClearDepthStencilValue{ clear_value.depth, clear_value.stencil };
		}
		else
			new_value.color = vk::ClearColorValue(clear_value.color.x, clear_value.color.y, clear_value.color.z, clear_value.color.w);
	}

	// Prepare render targets
	utl::vector<vk::RenderingAttachmentInfoKHR> color_attachments{};
	if (rendering_info.colorAttachments)
	{
		for (const auto& color_attachment_info : *rendering_info.colorAttachments)
		{
			auto& attachment_info = color_attachments.emplace_back();

			FVkTexture* texture{ nullptr };
			if (color_attachment_info.backbuffer)
				texture = &m_backbuffer[m_pDevice->m_swapchain.currentFrame];
			else
				texture = textures[color_attachment_info.handle].get();

			auto& target_subresource = texture->getSubresource(color_attachment_info.mipLevel);
			attachment_info.imageView = target_subresource.imageView;
			attachment_info.imageLayout = target_subresource.layout;

			attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
			attachment_info.loadOp = get_attachment_load_op(color_attachment_info.loadOp);
			attachment_info.storeOp = get_attachment_store_op(color_attachment_info.storeOp);
			attachment_info.clearValue = clear_values[color_attachment_info.handle];
		}
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if (rendering_info.depthAttachment && rendering_info.depthAttachment->handle != invalid_handle)
	{
		auto& depth_target = textures.at(rendering_info.depthAttachment->handle);
		auto& target_subresource = depth_target->getSubresource(rendering_info.depthAttachment->mipLevel);

		depth_attachment.imageView = target_subresource.imageView;
		depth_attachment.imageLayout = target_subresource.layout;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = get_attachment_load_op(rendering_info.depthAttachment->loadOp);
		depth_attachment.storeOp = get_attachment_store_op(rendering_info.depthAttachment->storeOp);
		depth_attachment.clearValue = clear_values[rendering_info.depthAttachment->handle];
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle && rendering_info.stencilAttachment->handle != invalid_handle)
	{
		auto& stencil_target = textures.at(rendering_info.stencilAttachment->handle);
		auto& target_subresource = stencil_target->getSubresource(rendering_info.stencilAttachment->mipLevel);

		stencil_attachment.imageView = target_subresource.imageView;
		stencil_attachment.imageLayout = target_subresource.layout;
		stencil_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		stencil_attachment.loadOp = get_attachment_load_op(rendering_info.stencilAttachment->loadOp);
		stencil_attachment.storeOp = get_attachment_store_op(rendering_info.stencilAttachment->storeOp);
		stencil_attachment.clearValue = clear_values[rendering_info.stencilAttachment->handle];
	}

	vk::RenderingInfoKHR begin_rendering_info{};
	begin_rendering_info.renderArea.offset.x = rendering_info.renderArea.offset.x;
	begin_rendering_info.renderArea.offset.y = rendering_info.renderArea.offset.y;
	begin_rendering_info.renderArea.extent.width = rendering_info.renderArea.size.x;
	begin_rendering_info.renderArea.extent.height = rendering_info.renderArea.size.y;
	begin_rendering_info.colorAttachmentCount = utl::cast<u32>(color_attachments.size());
	begin_rendering_info.pColorAttachments = color_attachments.data();
	begin_rendering_info.layerCount = rendering_info.layerCount;

	if (rendering_info.depthAttachment && rendering_info.depthAttachment->handle != invalid_handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures.at(rendering_info.depthAttachment->handle);
		if (!is_depth_only_format(depth_target->info.format) && rendering_info.stencilAttachment->handle == rendering_info.depthAttachment->handle)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment->handle != invalid_handle && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->DebugBeginLabel(current_buffer, rendering_info.name, 0x5555ffff);
#endif

	current_buffer.beginRenderingKHR(begin_rendering_info);
}

void CVkRenderContext::EndRendering(EQueueTypeFlagBits queue)
{
	auto& command_buffer = getLastCommandBuffer(queue);

	auto& current_buffer = command_buffer.get();
	current_buffer.endRenderingKHR();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->EndLabel(current_buffer);
#endif
}

void CVkRenderContext::BeginCompute(const FBeginComputeInfo& compute_info)
{
	auto& command_buffer = getLastCommandBuffer(compute_info.queue);
	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->DebugBeginLabel(current_buffer, compute_info.name, 0x55ff55ff);
#endif
}

void CVkRenderContext::EndCompute(EQueueTypeFlagBits queue)
{
	auto& command_buffer = getLastCommandBuffer(queue);
	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_pDevice->EndLabel(current_buffer);
#endif
}

void CVkRenderContext::processMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info)
{
	utl::vector<vk::ImageMemoryBarrier2KHR> image_barriers{};
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

	utl::vector<vk::BufferMemoryBarrier2KHR> buffer_barriers{};
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

	utl::vector<vk::MemoryBarrier2KHR> memory_barriers{};
	for (const auto& memory_info : translation_info.memoryTranslationInfos)
	{
		auto& memory_barrier = memory_barriers.emplace_back();

		memory_barrier.srcStageMask = memory_info.srcStageMask;
		memory_barrier.srcAccessMask = memory_info.srcAccessMask;

		memory_barrier.dstStageMask = memory_info.dstStageMask;
		memory_barrier.dstAccessMask = memory_info.dstAccessMask;
	}

	vk::DependencyInfoKHR dependency_info{};
	dependency_info.imageMemoryBarrierCount = utl::cast<u32>(image_barriers.size());
	dependency_info.pImageMemoryBarriers = image_barriers.data();
	dependency_info.bufferMemoryBarrierCount = utl::cast<u32>(buffer_barriers.size());
	dependency_info.pBufferMemoryBarriers = buffer_barriers.data();
	dependency_info.memoryBarrierCount = utl::cast<u32>(memory_barriers.size());
	dependency_info.pMemoryBarriers = memory_barriers.data();
	dependency_info.dependencyFlags = vk::DependencyFlagBits::eByRegion;

	auto& current_buffer = command_buffer.get();
	current_buffer.pipelineBarrier2KHR(dependency_info);
}

void CVkRenderContext::SetViewport(f32 width, f32 height, bool flip_viewport)
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

void CVkRenderContext::SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height)
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

// TODO: add indirect Draw support
void CVkRenderContext::Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info)
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
			current.drawIndexed(index_count, 1, range.beginIndex, range.beginVertex, 0);
		}
	}
}

void CVkRenderContext::Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();
	//current.drawMeshTasksIndirectEXT()

	auto& indirect_command_buffer = m_pResources->get<FVkBuffer>(draw_info.bufferId);
	auto& indirect_count_buffer = m_pResources->get<FVkBuffer>(draw_info.countBufferId);
	current.drawIndexedIndirectCount(indirect_command_buffer->handle, draw_info.offset, indirect_count_buffer->handle, draw_info.countOffset, draw_info.maxDrawCount, draw_info.stride);
	//current.drawIndexedIndirect(indirect_command_buffer->handle, draw_info.offset, draw_info.count, draw_info.stride);
}

void CVkRenderContext::Dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_info, u64 subgroup_index)
{
	vk::DispatchIndirectCommand command{};
	command.x = dispatch_info.globalSizes.x;
	command.y = dispatch_info.globalSizes.y;
	command.z = dispatch_info.globalSizes.z;

	if (dispatch_info.divideByLocals)
	{
		auto& shader = m_pResources->get<FVkShader>(dispatch_info.shaderHandle);
		auto& reflection = m_pResources->get<FVkShaderReflectionData>(shader->reflection);

		auto& localSizes = reflection->localSizes[subgroup_index];
		command.x = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.x) / utl::cast<f32>(localSizes[0]))), 1u);
		command.y = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.y) / utl::cast<f32>(localSizes[1]))), 1u);
		command.z = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.z) / utl::cast<f32>(localSizes[2]))), 1u);
	}

	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();
	current.dispatch(command.x, command.y, command.z);
}

void CVkRenderContext::Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info)
{
	
}

FDeviceLimitations CVkRenderContext::GetDeviceLimitations()
{
	FDeviceLimitations limits{};

	auto& logical = m_pDevice->m_logicalDevice.handle;

	return limits;
}

const FRenderFrameStats& CVkRenderContext::GetRenderFrameStats() const
{
	return m_renderStats;
}

EAdapterVendor CVkRenderContext::GetDeviceVendor() const
{
	auto& physical = m_pDevice->GetPhysicalDevice();

	//switch (physical.properties.vendorID)
	//{
	//case 0x10DE: return EAdapterVendor::eNVidia;
	//case 0x1002: return EAdapterVendor::eAMD;
	//case 0x8086: return EAdapterVendor::eIntel;
	//case 0x13B5: 
	//default: break;
	//}
	
	return EAdapterVendor::eNVidia;
}

u64 CVkRenderContext::GetTextureResource(const char* name)
{
	return m_pResources->getId<FVkTexture>(name);
}

u64 CVkRenderContext::GetShaderResource(const char* name)
{
	return m_pResources->getId<FVkShader>(name);
}

FPoint2D<u32> CVkRenderContext::GetRenderResolution() const
{
	FPoint2D<u32> resolution{};
	resolution.x = m_pDevice->m_swapchain.extent.width;
	resolution.y = m_pDevice->m_swapchain.extent.height;

	return resolution;
}

FRect2D<f32> CVkRenderContext::GetRenderRectFromRT(u64 texture_id)
{
	auto& texture = m_pResources->get<FVkTexture>(texture_id);

	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(texture->info.size.width);
	rect.size.y = utl::cast<f32>(texture->info.size.height);

	return rect;
}

FRect2D<f32> CVkRenderContext::GetScreenRenderRect()
{
	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(m_pDevice->m_swapchain.extent.width);
	rect.size.y = utl::cast<f32>(m_pDevice->m_swapchain.extent.height);
	return rect;
}

// TODO: update only targets, that have same size with swapchain
void CVkRenderContext::UpdateSurface(i32 surface_width, i32 surface_height)
{
	if (this->m_iSurfaceWidth != surface_width || this->m_iSurfaceHeight != surface_height)
	{
		this->m_iSurfaceWidth = surface_width;
		this->m_iSurfaceHeight = surface_height;

		auto& logical_device = m_pDevice->GetLogicalDevice();
		logical_device.handle.waitIdle();

		m_pDevice->CleanupSwapchain();
		m_pDevice->CreateSwapchain(surface_width, surface_height);
		m_pDevice->m_swapchain.currentFrame = 0u;

		auto& textures = m_pResources->data<FVkTexture>();

		// Updating render targets
		for (u64 texture_id = 0ull; texture_id < textures.size(); ++texture_id)
		{
			bool is_storage{ false };
			u64 swapchainFrameIndex{ 0ull };

			auto& texture = textures.at(texture_id);

			if (!texture->isRenderTarget || (texture->info.size.policy == ERenderTargetSizePolicy::eFixed))
				continue;

			for (auto& mip : texture->mips)
				m_pDevice->DestroyImageView(mip.imageView);
			texture->mips.clear();

			m_pDevice->DestroyImageView(texture->base.imageView);
			m_pDevice->DestroyImage(texture->image);

			FVkTextureCreateInfo texture_ci{};
			texture_ci.format = texture->info.format;
			texture_ci.typeFlags = texture->info.typeFlags;
			texture_ci.size = texture->info.size;
			texture_ci.mipLevels = texture->info.mipLevels;
			texture_ci.arrayLayers = texture->info.arrayLayers;
			texture_ci.CreateSampler = false;
#ifndef BUILD_WITHOUT_DEBUGGIND
			texture_ci.name = texture->name;
#endif
			// TODO: here i should set rendering resolution, not swapchain image size
			if (texture->info.size.policy == ERenderTargetSizePolicy::eScalable)
			{
				texture_ci.size.width = utl::cast<u32>(utl::cast<f32>(surface_width) * texture_ci.size.scale);
				texture_ci.size.height = utl::cast<u32>(utl::cast<f32>(surface_height)* texture_ci.size.scale);
			}

			check_api_result(CreateTexture(texture_ci, *texture));
			
			is_storage = utl::cast<bool>(texture_ci.typeFlags & ETextureCreateFlagBits::eStorage);

			//if (is_storage)
			//	m_textureLayoutInfos.emplace_back(texture_id);
		}

		// Update backbuffer resources
		updateBackbuffer();
	}
}

void CVkRenderContext::GetNativeContext(void* context)
{
	auto* native_context = static_cast<FVkNativeContext*>(context);
	auto& physical_device = m_pDevice->GetPhysicalDevice();
	auto& logical_device = m_pDevice->GetLogicalDevice();
	auto& swapchain = m_pDevice->getSwapchain();

	native_context->instance = m_pDevice->GetInstance();
	native_context->physicalDevice = physical_device.handle;
	native_context->device = logical_device.handle;

	if(auto found = physical_device.queueFamilies.queueToFamily.find(EQueueTypeFlagBits::ePresent); found != physical_device.queueFamilies.queueToFamily.end())
		native_context->presentQueueFamily = found->second;
	if(auto found = logical_device.queues.find(EQueueTypeFlagBits::ePresent); found != logical_device.queues.end())
	native_context->presentQueue = found->second;

	native_context->pipelineCache = m_pDevice->GetPipelineCache();
	native_context->backbufferFormat = swapchain.format;
	native_context->backbufferFrames = swapchain.framesInFlight;
	native_context->samples = m_pDevice->GetMultisamplingCount();
	native_context->allocationCallbacks = m_pDevice->GetAllocationCallbacks();
	native_context->memoryAllocator = m_pDevice->GetMemoryAllocator();
}

FVkCommandBuffer& CVkRenderContext::CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[command_buffer_ci.mask];

	if (!per_queue.commandPool.handle)
	{
		FVkCommandPoolCreateInfo pool_ci{};
		pool_ci.mask = command_buffer_ci.mask;

		check_api_result(m_pDevice->CreateCommandPool(pool_ci, per_queue.commandPool));
	}
	
	auto& command_buffer = per_queue.commandBuffers.emplace_back();
	check_api_result(m_pDevice->CreateCommandBuffer(command_buffer_ci, per_queue.commandPool, command_buffer));

	return command_buffer;
}

FVkCommandBuffer& CVkRenderContext::getLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers.back();
}

FVkCommandBuffer& CVkRenderContext::GetCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers.at(index);
}

utl::vector<FVkCommandBuffer>& CVkRenderContext::getCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers;
}

void CVkRenderContext::DestroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = m_perThreadContext[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	auto& command_buffer = per_queue.commandBuffers.back();
	m_pDevice->DestroyCommandBuffer(per_queue.commandPool, command_buffer);
	per_queue.commandBuffers.pop_back();
}

const std::unique_ptr<CVkRenderDevice>& CVkRenderContext::getDevice() const
{
	return m_pDevice;
}

void CVkRenderContext::BeginFrame()
{
	u32 image_index{ 0u };

	ProcessResourceBarriers();
	processResourceUpload();
	// TODO: sync queues here

	vk::Result res{};
	try { res = m_pDevice->AcquireNextImage(image_index); }
	catch (vk::OutOfDateKHRError error) { res = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("Failed to acquire swap chain image!"); }

	if (res == vk::Result::eErrorOutOfDateKHR)
		UpdateSurface(m_iSurfaceWidth, m_iSurfaceHeight);
	else if (res == vk::Result::eSuboptimalKHR)
		log_error("Failed to acquire swap chain image!");

	if (res != vk::Result::eSuccess)
		return;

	m_bIsFrameStarted = true;

	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	m_pDevice->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eRenderPassContinue, image_index);
	m_pDevice->ResetTimestampQuery(m_timestampQuery);
	//m_pDevice->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eTopOfPipe, m_timestampQuery);

	// Update bindless textures
	auto& textures = m_pResources->data<FVkTexture>();

	if (m_textureIndexRemap.empty())
	{
		// Resizing remap vector
		m_textureIndexRemap.resize(textures.size());

		u64 image_offset{ 0ull };
		for (u64 texture_index = 0ull; texture_index < textures.size(); ++texture_index)
		{
			auto& texture = textures.at(texture_index);
			if (texture->isRenderTarget)
				continue;

			m_textureIndexRemap.at(texture_index) = image_offset++;

			for (u32 frame_index = 0u; frame_index < m_pDevice->m_swapchain.framesInFlight; ++frame_index)
				m_textureToUpdate[frame_index].push(texture_index);
		}
	}
}

void CVkRenderContext::EndFrame()
{
	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	//m_pDevice->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eBottomOfPipe, m_timestampQuery);

	m_pDevice->EndCommandBuffer(command_buffer);
	m_bIsFrameStarted = false;

	vk::Result presentResult{};
	try { presentResult = m_pDevice->Present(command_buffer); }
	catch (vk::OutOfDateKHRError err) { presentResult = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("failed to present swap chain image!"); }

	utl::vector<f32> results{};
	auto timestamp_result = m_pDevice->GetTimestampQueryResults(m_timestampQuery, m_renderStats.timestampResults);
	if (!timestamp_result.fault)
	{
		m_renderStats.gpuFrameTime = 0.f;
		for (const auto& time : m_renderStats.timestampResults)
			m_renderStats.gpuFrameTime += time;
	}

	// Get gpu memory usage info
	m_pDevice->GetOccupedDeviceMemory(vk::MemoryHeapFlagBits::eDeviceLocal, m_renderStats.totalGPUMemory, m_renderStats.usedGPUMemory);

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		UpdateSurface(m_iSurfaceWidth, m_iSurfaceHeight);
	utl::hash_map<std::string, std::string> kk;
	
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
	// TODO: i don't need to set layout back to undefined for one time targets or for periodical targets
	auto& textures = m_pResources->data<FVkTexture>();
	for (auto& texture : textures)
	{
		if (texture->isRenderTarget && !(texture->info.typeFlags & ETextureCreateFlagBits::eStorage))
		{
			// Reset layout for all mip level
			for(auto& mip : texture->mips)
				mip.layout = vk::ImageLayout::eUndefined;

			// Also reset view that stores all other levels
			texture->base.layout = vk::ImageLayout::eUndefined;
		}
	}

	// Also for backbuffer
	for (auto& current : m_backbuffer)
	{
		for (auto& mip : current.mips)
			mip.layout = vk::ImageLayout::eUndefined;

		current.base.layout = vk::ImageLayout::eUndefined;
	}

	m_utilizationCounter++;
}

bool CVkRenderContext::IsSwapChainFinished()
{
	return (m_utilizationCounter % m_pDevice->m_swapchain.framesInFlight == 0) && m_utilizationCounter != 0ull;
}

void CVkRenderContext::BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->DebugBeginLabel(current, label, color);
}

void CVkRenderContext::InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->DebugInsertLabel(current, label, color);
}

void CVkRenderContext::EndDebugLabel(EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = getLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	m_pDevice->EndLabel(current);
}

void CVkRenderContext::destroyThreadContext(FVkThreadContext& context)
{
	// Destroy command buffers and pools
	for (auto& [queue_id, queue_resources] : context.perQueue)
	{
		for (auto& command_buffer : queue_resources.commandBuffers)
			m_pDevice->DestroyCommandBuffer(queue_resources.commandPool, command_buffer);

		m_pDevice->DestroyCommandPool(queue_resources.commandPool);
	}

	// Destroy descriptor sets
	for (auto& [id, descriptor_set] : context.descriptorSets)
		m_pDevice->DestroyDescriptorSet(context.descriptorPool, descriptor_set);

	// Destroy descriptor pool
	m_pDevice->DestroyDescriptorPool(context.descriptorPool);
}

void CVkRenderContext::ProcessResourceBarriers()
{
	// TODO: add mip level and array layer support
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

	auto& command_buffer = CreateCommandBuffer(command_buffer_ci);

	m_pDevice->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	FVkTranslationInfo translation_info{};
	for (const auto& layout_info : m_textureLayoutInfos)
	{
		auto& texture = textures.at(layout_info.textureId);
		auto& target_subresource = texture->getSubresource();

		if (texture->info.typeFlags & ETextureCreateFlagBits::eStorage)
		{
			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
			target_subresource.layout = texture_translation_info.newLayout = vk::ImageLayout::eGeneral;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);
		}
	}

	processMemoryBarriers(command_buffer, translation_info);

	m_pDevice->EndCommandBuffer(command_buffer);
	m_pDevice->SubmitCommandBuffer(command_buffer);

	DestroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

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

	auto& command_buffer = CreateCommandBuffer(command_buffer_ci);

	m_pDevice->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	utl::vector<FVkBuffer> staging_buffers{};

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
			m_pDevice->CreateBuffer(staging_buffer_ci, staging_buffer);

			// Copy data to buffer
			void* mapped_buffer_memory{ nullptr };
			m_pDevice->MapBufferMemory(staging_buffer, &mapped_buffer_memory);
			std::memcpy(mapped_buffer_memory, texture_upload.data.data(), texture_upload.data.size());
			m_pDevice->UnmapBufferMemory(staging_buffer);

			texture_upload.data.clear();
		}

		// Translate all images to transfer dst stage
		utl::vector<u64> generate_mip_textures{};
		FVkTranslationInfo translation_info{};
		for (const auto& texture_upload : m_textureUploadInfos)
		{
			auto& texture = textures.at(texture_upload.textureId);
			auto& target_subresource = texture->getSubresource();

			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
			target_subresource.layout = texture_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);

			if (texture->info.mipLevels > 1u && texture_upload.mipLayerOffsets.empty())
				generate_mip_textures.emplace_back(texture_upload.textureId);
		}

		// Process layout transfers
		processMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();

		// Copy buffer to image
		for (u64 idx = 0ull; idx < m_textureUploadInfos.size(); ++idx)
		{
			auto& texture_upload = m_textureUploadInfos.at(idx);
			auto& texture = textures.at(texture_upload.textureId);
			auto& staging_buffer = staging_buffers.at(idx);

			auto aspect_mask = get_texture_aspect_flags(*texture);

			utl::vector<vk::BufferImageCopy2KHR> copy_regions{};
			if (!texture_upload.mipLayerOffsets.empty())
			{
				for (u32 layer = 0u; layer < texture->totalLayers; ++layer)
				{
					auto& layer_offsets = texture_upload.mipLayerOffsets.at(layer);
					for (u32 mip = 0u; mip < texture->info.mipLevels; ++mip)
					{
						u64 offset = layer_offsets.at(mip);
						auto& copy_region = copy_regions.emplace_back();
						copy_region.imageSubresource.aspectMask = aspect_mask;
						copy_region.imageSubresource.mipLevel = mip;
						copy_region.imageSubresource.baseArrayLayer = layer;
						copy_region.imageSubresource.layerCount = texture->info.arrayLayers;
						copy_region.imageExtent.width = texture->info.size.width >> mip;
						copy_region.imageExtent.height = texture->info.size.height >> mip;
						copy_region.imageExtent.depth = texture->info.size.depth;
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
				copy_region.imageExtent.width = texture->info.size.width;
				copy_region.imageExtent.height = texture->info.size.height;
				copy_region.imageExtent.depth = texture->info.size.depth;
				copy_region.bufferOffset = 0ull;
			}

			copyBufferToTexture(command_buffer, staging_buffer, *texture, copy_regions);
		}

		auto& current_command_buffer = command_buffer.get();
		for (auto& texture_id : generate_mip_textures)
		{
			auto& texture = textures.at(texture_id);
			auto aspect_mask = get_texture_aspect_flags(*texture);

			u32 mip_width{ texture->info.size.width };
			u32 mip_heigth{ texture->info.size.height };

			FVkImageTranslationInfo image_translation_info{};
			image_translation_info.handle = texture->image.handle;
			image_translation_info.subresourceRange.baseArrayLayer = 0u;
			image_translation_info.subresourceRange.levelCount = 1;
			image_translation_info.subresourceRange.layerCount = texture->totalLayers;
			image_translation_info.subresourceRange.aspectMask = aspect_mask;

			for (u32 mip = 1u; mip < texture->info.mipLevels; ++mip)
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
				image_blit.srcOffsets[1] = vk::Offset3D{ utl::cast<i32>(mip_width), utl::cast<i32>(mip_heigth), 1 };
				image_blit.srcSubresource.aspectMask = aspect_mask;
				image_blit.srcSubresource.mipLevel = image_translation_info.subresourceRange.baseMipLevel;
				image_blit.srcSubresource.baseArrayLayer = 0u;
				image_blit.srcSubresource.layerCount = 1u;
				image_blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
				image_blit.dstOffsets[1] = vk::Offset3D{ utl::cast<i32>(mip_width > 1u ? mip_width / 2u : 1u), utl::cast<i32>(mip_heigth > 1u ? mip_heigth / 2u : 1u), 1 };
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
			auto& texture = textures.at(texture_upload.textureId);
			auto& target_subresource = texture->getSubresource();

			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			target_subresource.layout = texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);
		}

		processMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();
	}

	// Upload buffers
	for(const auto& upload_info : m_bufferUploadInfos)
	{
		auto& physical = m_pDevice->GetPhysicalDevice();
		utl::vector<vk::BufferCopy2KHR> copy_regions(upload_info.regions.size());

		auto& dst_buffer = m_pResources->get<FVkBuffer>(upload_info.bufferId);
		
		// Calculate elements to copy count
		u64 staging_count{ 0ull };
		u64 bytes_count{ 0ull };
		for (u64 idx = 0ull; idx < copy_regions.size(); ++idx)
		{
			auto& region_info = upload_info.regions[idx];

			auto& copy_region = copy_regions[idx];
			copy_region.srcOffset = region_info.srcOffset;
			copy_region.dstOffset = region_info.dstOffset;
			copy_region.size = region_info.size;

			staging_count += region_info.count;
			bytes_count += region_info.size;
		}

		// Create new staging buffer
		FVkBufferCreateInfo vk_buffer_ci{};
		vk_buffer_ci.instanceCount = 1;
		vk_buffer_ci.instanceSize = bytes_count;
		vk_buffer_ci.minOffsetAlignment = 1;
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuOnly;
		vk_buffer_ci.usageFlags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
#ifndef BUILD_WITHOUT_DEBUGGIND
		vk_buffer_ci.name = std::format("staging for #{}", upload_info.bufferId);
#endif

		auto src_buffer = staging_buffers.emplace_back();
		m_pDevice->CreateBuffer(vk_buffer_ci, src_buffer);

		// Copy data to staging buffer
		void* mapped_memory{ nullptr };
		m_pDevice->MapBufferMemory(src_buffer, &mapped_memory);

		// Copy upload data to buffer
		char* dst_memory = utl::cast<char*>(mapped_memory);
		std::memcpy(dst_memory, upload_info.data.data(), upload_info.data.size());
		m_pDevice->UnmapBufferMemory(src_buffer);

		vk::CopyBufferInfo2KHR copy_buffer{};
		copy_buffer.srcBuffer = src_buffer.handle;
		copy_buffer.dstBuffer = dst_buffer->handle;
		copy_buffer.regionCount = utl::cast<u32>(copy_regions.size());
		copy_buffer.pRegions = &copy_regions[0];
		
		auto& current = command_buffer.get();
		current.copyBuffer2KHR(copy_buffer);
	}

	m_pDevice->EndCommandBuffer(command_buffer);

	m_pDevice->SubmitCommandBuffer(command_buffer);

	// TODO: submit command buffer. But with pushing semaphores for synchronize with main command buffer
	DestroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

	// destroy staging buffers
	for (auto& staging_buffer : staging_buffers)
		m_pDevice->DestroyBuffer(staging_buffer);

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
			DestroyTexture(*texture);
			m_pResources->remove<FVkTexture>(destroy_resource.id);
		}
		else if (destroy_resource.mask & EResourceFlagBits::eShader)
		{
			auto& shader = m_pResources->get<FVkShader>(destroy_resource.id);
			DestroyShader(*shader);
			m_pResources->remove<FVkShader>(destroy_resource.id);
		}
		else if (destroy_resource.mask & EResourceFlagBits::eBuffer)
		{
			auto& buffer = m_pResources->get<FVkBuffer>(destroy_resource.id);
			m_pDevice->DestroyBuffer(*buffer);
			m_pResources->remove<FVkBuffer>(destroy_resource.id);
		}

		m_destroyResource.pop();
	}
}

void CVkRenderContext::updateBackbuffer()
{
	auto& swapchain = m_pDevice->m_swapchain;

	m_backbuffer.clear();
	m_backbuffer.resize(swapchain.framesInFlight);

	for (u64 frame = 0ull; frame < m_backbuffer.size(); ++frame)
	{
		auto& current = m_backbuffer[frame];
		current.isAlive = true;
		current.isRenderTarget = true;
		current.name = "backbuffer_image";
		current.image.handle = swapchain.images[frame];

		auto& mip = current.mips.emplace_back();
		mip.imageView = current.base.imageView = swapchain.imageViews[frame];

		current.info.typeFlags = ETextureCreateFlagBits::e2D;
		current.info.format = swapchain.format;
		current.info.size.width = swapchain.extent.width;
		current.info.size.height = swapchain.extent.height;
	}
}

FVkBufferCreateInfo CVkRenderContext::makeBufferCreateInfo(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto& physical = m_pDevice->GetPhysicalDevice();

	FVkBufferCreateInfo vk_buffer_ci{};
	vk_buffer_ci.instanceCount = buffer_ci.count;
	vk_buffer_ci.instanceSize = buffer_ci.size;

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_buffer_ci.name = name;
#endif

	// Build buffer usage flags
	u64 minimal_alignment{ 1ull };
	if (buffer_ci.flags & EBufferUsageFlagBits::eTransfer)
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;

	if (buffer_ci.flags & EBufferUsageFlagBits::eVertex)
	{
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eVertexBuffer;
		minimal_alignment = std::max(minimal_alignment, 16ull);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eIndex)
	{
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eIndexBuffer;
		minimal_alignment = std::max(minimal_alignment, 4ull);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eUniform)
	{
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;
		minimal_alignment = std::lcm(physical.properties.limits.minUniformBufferOffsetAlignment, physical.properties.limits.nonCoherentAtomSize);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eStorage)
	{
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eStorageBuffer;
		minimal_alignment = std::max(minimal_alignment, physical.properties.limits.minStorageBufferOffsetAlignment);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eIndirect)
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eIndirectBuffer;

	if (buffer_ci.flags & EBufferUsageFlagBits::eHasDeviceAddress)
		vk_buffer_ci.usageFlags |= vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

	// Determine memory type
	if (buffer_ci.flags & EBufferUsageFlagBits::eGpuOnly)
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eUnknown;
	else if (buffer_ci.flags & EBufferUsageFlagBits::eCpuToGpu)
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuToGpu;
	else
		vk_buffer_ci.memoryUsage = vma::MemoryUsage::eCpuOnly;

	// Add buffer alignment
	vk_buffer_ci.minOffsetAlignment = minimal_alignment;// std::lcm(minimal_alignment, physical.properties.limits.nonCoherentAtomSize);

	return vk_buffer_ci;
}

FRenderApiErrorInfo CVkRenderContext::CreateTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture)
{
	texture.info.format = texture_ci.format;
	texture.info.typeFlags = texture_ci.typeFlags;
	texture.info.size = texture_ci.size;
	texture.info.mipLevels = texture_ci.mipLevels;
	texture.info.arrayLayers = texture_ci.arrayLayers;
	
	FVkImageCreateInfo image_ci{};
	FVkImageViewCreateInfo image_view_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	texture.name = image_ci.name = texture_ci.name;
#endif

	if (texture_ci.size.depth > 1 || (texture_ci.typeFlags & ETextureCreateFlagBits::e3D))
		image_ci.ci.imageType = vk::ImageType::e3D;
	else
	{
		if (texture_ci.typeFlags & ETextureCreateFlagBits::e1D)
			image_ci.ci.imageType = vk::ImageType::e1D;
		else if (texture_ci.typeFlags & ETextureCreateFlagBits::e2D)
			image_ci.ci.imageType = vk::ImageType::e2D;
		else if (texture_ci.typeFlags & ETextureCreateFlagBits::e3D)
			image_ci.ci.imageType = vk::ImageType::e3D;
		else
			image_ci.ci.imageType = vk::ImageType::e2D;
	}

	image_ci.ci.extent = vk::Extent3D{ texture_ci.size.width, texture_ci.size.height, texture_ci.size.depth };
	image_ci.ci.mipLevels = texture_ci.mipLevels;

	image_ci.ci.arrayLayers = texture_ci.arrayLayers;
	if ((texture_ci.typeFlags & ETextureCreateFlagBits::eArray) && (texture_ci.typeFlags & ETextureCreateFlagBits::eCubemap))
	{
		image_ci.ci.arrayLayers *= 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	}
	else if (texture_ci.typeFlags & ETextureCreateFlagBits::eArray)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if (texture_ci.typeFlags & ETextureCreateFlagBits::eCubemap)
	{
		image_ci.ci.arrayLayers = 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	}
	else if (texture_ci.typeFlags & ETextureCreateFlagBits::e3D)
		image_view_ci.ci.viewType = vk::ImageViewType::e3D;
	else
	{
		image_ci.ci.arrayLayers = 1u;
		image_view_ci.ci.viewType = vk::ImageViewType::e2D;
	}

	image_view_ci.ci.subresourceRange.layerCount = texture.totalLayers = image_ci.ci.arrayLayers;

	auto usageFlags = get_texture_usage_flags(texture);

	image_ci.memoryUsage = vma::MemoryUsage::eUnknown;
	image_view_ci.ci.format = image_ci.ci.format = utl::cast<vk::Format>(texture_ci.format);
	image_ci.ci.initialLayout = vk::ImageLayout::eUndefined;
	image_ci.ci.usage = usageFlags;
	texture.info.sampleCount = image_ci.ci.samples = texture_ci.sampleCount;
	image_ci.ci.sharingMode = vk::SharingMode::eExclusive;
	image_ci.ci.flags = (texture_ci.typeFlags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(m_pDevice->CreateImage(image_ci, texture.image));

	auto image_aspect = get_texture_aspect_flags(texture);
	if (image_aspect & vk::ImageAspectFlagBits::eStencil)
		image_aspect = vk::ImageAspectFlagBits::eDepth;

	image_view_ci.ci.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	image_view_ci.ci.subresourceRange.aspectMask = image_aspect;
	image_view_ci.ci.image = texture.image.handle;
	image_view_ci.ci.subresourceRange.baseArrayLayer = 0u;
	image_view_ci.ci.subresourceRange.layerCount = texture.totalLayers;

	// At first Create default view that contain all subresources
	image_view_ci.ci.subresourceRange.baseMipLevel = 0u;
	image_view_ci.ci.subresourceRange.levelCount = texture.base.mipLevelCount = texture_ci.mipLevels;

#ifndef BUILD_WITHOUT_DEBUGGIND
	image_view_ci.name = std::format("{}_main", texture_ci.name);
#endif

	RETURN_WHEN_FAULT(m_pDevice->CreateImageView(image_view_ci, texture.base.imageView));
	
	texture.mips.resize(texture_ci.mipLevels);
	for (u32 mip_index = 0u; mip_index < texture_ci.mipLevels; ++mip_index)
	{
		auto& current_mip = texture.mips.at(mip_index);
		image_view_ci.ci.subresourceRange.baseMipLevel = current_mip.baseMipLevel = mip_index;
		image_view_ci.ci.subresourceRange.levelCount = current_mip.mipLevelCount = 1u;

#ifndef BUILD_WITHOUT_DEBUGGIND
		image_view_ci.name = std::format("{}[{}]", texture_ci.name, mip_index);
#endif
		RETURN_WHEN_FAULT(m_pDevice->CreateImageView(image_view_ci, current_mip.imageView));
	}

	FRenderApiErrorInfo res = make_api_error(vk::ObjectType::eSampler, vk::Result::eSuccess);
	if (texture_ci.CreateSampler)
	{
		FVkSamplerCreateInfo sampler_ci{};
		sampler_ci.addressMode = texture_ci.addressMode;
		sampler_ci.enableAnisotropy = texture_ci.enableAnisotropy;
		sampler_ci.enableCompareOp = is_depth_format(image_ci.ci.format);
		sampler_ci.minFilter = sampler_ci.magFilter = texture_ci.filter;
		sampler_ci.mipLevels = texture_ci.mipLevels;
#ifndef BUILD_WITHOUT_DEBUGGIND
		sampler_ci.name = texture_ci.name;
#endif
		u64 key = calculate_sampler_hash(sampler_ci);
		auto& sampler = m_samplerCache[key];
		
		if(!sampler)
			res = m_pDevice->CreateSampler(sampler_ci, sampler);

		texture.sampler = key;
	}

	texture.isAlive = true;

	return res;
}

void CVkRenderContext::DestroyTexture(FVkTexture& texture)
{	
	for(auto& mip : texture.mips)
		m_pDevice->DestroyImageView(mip.imageView);
	texture.mips.clear();

	m_pDevice->DestroyImageView(texture.base.imageView);
	m_pDevice->DestroyImage(texture.image);

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
		RETURN_WHEN_FAULT(m_pDevice->CreateDescriptorSetLayout(descriptor_set_layout_ci, descriptor_set_Layout));
	}

	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = resources_info.name;
#endif
	pool_ci.poolSizes = reflection->descriptorPoolSizes;
	pool_ci.usageCount = resources_info.usageCount * m_pDevice->m_swapchain.framesInFlight;
	RETURN_WHEN_FAULT(m_pDevice->CreateDescriptorPool(pool_ci, shader.descriptorPool));

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
		RETURN_WHEN_FAULT(m_pDevice->CreateDescriptorSet(descriptor_set_ci, descriptor_set));
	}

	// Minimal buffer offset
	auto& physical = m_pDevice->GetPhysicalDevice();
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
		RETURN_WHEN_FAULT(m_pDevice->CreateBuffer(buffer_ci, *buffer));


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

	// Send descriptor set layouts to Create info
	for (u64 idx = 1ull; idx < shader.descriptorSetLayouts.size() + 1ull; ++idx)
	{
		if(shader.descriptorSetLayouts.count(idx))
			layout_ci.descriptorSetLayouts.emplace_back(shader.descriptorSetLayouts[idx]);
	}
	//for (const auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
	//	layout_ci.descriptorSetLayouts.emplace_back(descriptor_set_layout);

	// Prepare push blocks info
	u32 current_offset{ 0u };
	for (const auto& [name, block] : reflection->pushBlocks)
	{
		auto& push_constant_range = layout_ci.pushConstantRanges.emplace_back();
		push_constant_range.stageFlags = block.stageFlags;
		push_constant_range.offset = current_offset;
		push_constant_range.size = utl::cast<u32>(block.size);
		current_offset += push_constant_range.size;
	}

	return m_pDevice->CreatePipelineLayout(layout_ci, shader.layout);
}

// Destroy shader object
void CVkRenderContext::DestroyShader(FVkShader& shader)
{
	for (auto& pipeline : shader.handles)
		m_pDevice->DestroyPipeline(pipeline);

	for (auto& shader_module : shader.shaderModules)
		m_pDevice->DestroyShaderModule(shader_module);

	for (auto& [set, descriptor_set] : shader.descriptorSets)
		m_pDevice->DestroyDescriptorSet(shader.descriptorPool, descriptor_set);

	for (auto& [id, buffer_id] : shader.uniformBuffers)
	{
		auto& buffer = m_pResources->get<FVkBuffer>(buffer_id);
		m_pDevice->DestroyBuffer(*buffer);
		m_pResources->remove<FVkBuffer>(buffer_id);
	}

	m_pDevice->DestroyDescriptorPool(shader.descriptorPool);

	m_pDevice->DestroyPipelineLayout(shader.layout);

	for (auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		m_pDevice->DestroyDescriptorSetLayout(descriptor_set_layout);

	shader.isAlive = false;
}

void CVkRenderContext::bindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader)
{
	utl::vector<vk::DescriptorSet> bindDescriptorSets{};

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
		current_command_buffer.bindDescriptorSets(shader.bindPoint, shader.layout, 0, utl::cast<u32>(bindDescriptorSets.size()), bindDescriptorSets.data(), 0u, nullptr);
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

void CVkRenderContext::copyBufferToTexture(const FVkCommandBuffer& command_buffer, const FVkBuffer& buffer, const FVkTexture& texture, const utl::vector<vk::BufferImageCopy2KHR>& copy_regions)
{
	auto& current_buffer = command_buffer.get();
	auto& target_subresource = texture.getSubresource();

	vk::CopyBufferToImageInfo2KHR copy_info{};
	copy_info.srcBuffer = buffer.handle;
	copy_info.dstImage = texture.image.handle;
	copy_info.dstImageLayout = target_subresource.layout;
	copy_info.regionCount = utl::cast<u32>(copy_regions.size());
	copy_info.pRegions = copy_regions.data();

	current_buffer.copyBufferToImage2KHR(&copy_info);
}
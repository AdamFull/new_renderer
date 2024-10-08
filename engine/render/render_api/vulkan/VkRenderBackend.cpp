#include <engine/render/render_api/vulkan/VkRenderBackend.h>
#include <engine/render/render_api/vulkan/VkCommon.h>

#include <util/Logger.h>

#include <numeric>
#include <type_traits>
#include <unordered_set>

#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/Buffer.h>

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
	return std::vformat(fmt, std::make_format_args(object_handle, args...));
}

BufferBuilder MakeBufferBuilder(const FBufferCreateInfo& buffer_ci, const vk::PhysicalDeviceProperties& properties)
{
	u64 minimal_alignment{ 1ull };
	vma::MemoryUsage memory_usage_flags{};
	vk::BufferUsageFlags usage_flags{};

	if (buffer_ci.flags & EBufferUsageFlagBits::eTransfer)
		usage_flags |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;

	if (buffer_ci.flags & EBufferUsageFlagBits::eVertex)
	{
		usage_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
		minimal_alignment = std::max(minimal_alignment, 16ull);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eIndex)
	{
		usage_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
		minimal_alignment = std::max(minimal_alignment, 4ull);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eUniform)
	{
		usage_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
		minimal_alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eStorage)
	{
		usage_flags |= vk::BufferUsageFlagBits::eStorageBuffer;
		minimal_alignment = std::max(minimal_alignment, properties.limits.minStorageBufferOffsetAlignment);
	}

	if (buffer_ci.flags & EBufferUsageFlagBits::eIndirect)
		usage_flags |= vk::BufferUsageFlagBits::eIndirectBuffer;

	if (buffer_ci.flags & EBufferUsageFlagBits::eHasDeviceAddress)
		usage_flags |= vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

	// Determine memory type
	if (buffer_ci.flags & EBufferUsageFlagBits::eGpuOnly)
		memory_usage_flags = vma::MemoryUsage::eUnknown;
	else if (buffer_ci.flags & EBufferUsageFlagBits::eCpuToGpu)
		memory_usage_flags = vma::MemoryUsage::eCpuToGpu;
	else
		memory_usage_flags = vma::MemoryUsage::eCpuOnly;

	// Add buffer alignment
	// std::lcm(minimal_alignment, physical.properties.limits.nonCoherentAtomSize);
	auto builder = BufferBuilder(buffer_ci.size, buffer_ci.count, minimal_alignment);
	builder.SetVmaUsage(memory_usage_flags);
	builder.SetUsage(usage_flags);

	return builder;
}

VkRenderBackend::~VkRenderBackend()
{
	auto& logical_device = render_device->GetLogicalDevice();
	Handle(logical_device).waitIdle();

	// Destroy thread contexts
	for (auto& [thread_id, context] : per_thread_context)
		DestroyThreadContext(context);

	// Destroy all textures
	auto& textures = render_resources->data<FVkTexture>();
	for (resource_handle_t texture_id = 0ull; texture_id < textures.size(); ++texture_id)
	{
		auto& texture = textures[texture_id];
		if (!texture->isAlive)
			continue;

		DestroyTexture(*texture);
	}

	// Destroy all shaders
	auto& shaders = render_resources->data<FVkShader>();
	for (resource_handle_t shader_id = 0ull; shader_id < shaders.size(); ++shader_id)
	{
		auto& shader = shaders[shader_id];
		if (!shader->isAlive)
			continue;

		DestroyShader(*shader);
	}

	// Destroy all buffers
	auto& buffers = render_resources->data<Buffer>();
	buffers.clear();

	render_device->DestroySampler(default_sampler);

	for(auto& [uid, sampler] : sampler_cache)
		render_device->DestroySampler(sampler);
}

void VkRenderBackend::Create(const FRenderContextCreateInfo& context_ci)
{
	render_device = std::make_unique<RenderDevice>();
	render_device->Create(context_ci);

	auto& swapchain = render_device->GetSwapchain();

	surface_width = swapchain.extent.width;
	m_iSurfaceHeight = swapchain.extent.height;

	render_resources = std::make_unique<vk_resource_container_t>();

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = render_device->swapchain.framesInFlight;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "Graphics";
#endif

	CreateCommandBuffer(command_buffer_ci);

	auto res = render_device->CreateTimestampQuery(256, timestamp_query);

	auto& per_thread = per_thread_context[std::this_thread::get_id()];
	auto& per_queue = per_thread.perQueue[EQueueTypeFlagBits::eGraphics];
	render_device->InitProfiler(per_queue.commandPool);

	UpdateBackbuffer();

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

	render_device->CreateSampler(sampler_ci, default_sampler);
}

void VkRenderBackend::CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci)
{
	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
	pool_ci.poolSizes = { { vk::DescriptorType::eCombinedImageSampler, k_max_bindless_resources }, {vk::DescriptorType::eUniformBuffer, 3u} };
	pool_ci.usageCount = 1u * render_device->swapchain.framesInFlight;
	pool_ci.flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = "global:textures";
#endif

	render_device->CreateDescriptorPool(pool_ci, global_descriptor.descriptorPool);

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
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind );
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind );
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind );
	bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind );

	vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCIEXT{};
	descriptorSetLayoutBindingFlagsCIEXT.bindingCount = utl::cast<u32>(bindingFlags.size());
	descriptorSetLayoutBindingFlagsCIEXT.pBindingFlags = bindingFlags.data();

	set_layout_ci.next = &descriptorSetLayoutBindingFlagsCIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_layout_ci.name = "global:textures";
#endif

	render_device->CreateDescriptorSetLayout(set_layout_ci, global_descriptor.deecriptorSetLayout);

	// Create descriptor set
	FVkDescriptorSetCreateInfo set_ci{};
	set_ci.descriptorCount = render_device->swapchain.framesInFlight;
	set_ci.layout = global_descriptor.deecriptorSetLayout;
	set_ci.pool = global_descriptor.descriptorPool.handle;

	// TODO: Potential problems here in future. I think that max_sets array is incorrect
	utl::vector<u32> max_sets(render_device->swapchain.framesInFlight, k_max_bindless_resources - 1);
	vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT descriptor_set_variable_descriptor_count_AIEXT{};
	descriptor_set_variable_descriptor_count_AIEXT.descriptorSetCount = render_device->swapchain.framesInFlight;
	descriptor_set_variable_descriptor_count_AIEXT.pDescriptorCounts = max_sets.data();
	set_ci.next = &descriptor_set_variable_descriptor_count_AIEXT;

#ifndef BUILD_WITHOUT_DEBUGGIND
	set_ci.name = "global:textures";
#endif

	render_device->CreateDescriptorSet(set_ci, global_descriptor.descriptorSet);

	// Create uniform buffers
	auto& physical = render_device->GetPhysicalDevice();
	auto minimal_alignment = std::lcm(physical.GetProperties().limits.minUniformBufferOffsetAlignment, physical.GetProperties().limits.nonCoherentAtomSize);

	for (auto& [uniform_name, size] : uniform_ci.uniforms)
	{
		auto builder = BufferBuilder{ size, render_device->swapchain.framesInFlight, minimal_alignment};
		builder.SetVmaUsage(vma::MemoryUsage::eCpuToGpu);
		builder.SetUsage(vk::BufferUsageFlagBits::eUniformBuffer);

#ifndef BUILD_WITHOUT_DEBUGGIND
		auto* debug_name = debug_mapping::find(uniform_name);

		if(debug_name)
			builder.SetDebugName(std::format("global:{}", debug_name));
		else
			builder.SetDebugName(std::format("global:{:x}", uniform_name));
#endif

		auto buffer = builder.BuildUnique(*render_device);

		auto buffer_name = create_object_unique_name("global:{:x}:{:x}", Handle(*buffer), uniform_name);
		auto buffer_id = render_resources->add<Buffer>(buffer_name.c_str(), std::move(buffer));
		global_descriptor.uniformBuffers.emplace(uniform_name, buffer_id);
	}
}

void VkRenderBackend::UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view)
{
	auto& dest_set = global_descriptor.descriptorSet.handles[render_device->swapchain.currentFrame];

	std::array<vk::DescriptorImageInfo, k_max_texture_update_per_frame> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, k_max_buffer_update_per_frame> buffer_descriptors{};
	utl::vector<vk::WriteDescriptorSet> write_descriptor_sets{};

	auto& textures = render_resources->data<FVkTexture>();

	// Bind and update global uniform
	u64 buffer_count{ 0ull };
	for (const auto& [uniform_name, uniform_data] : data_view.uniforms)
	{
		auto found_buffer = global_descriptor.uniformBuffers.find(uniform_name);
		if (found_buffer == global_descriptor.uniformBuffers.end())
			continue;

		auto& buffer = render_resources->get<Buffer>(found_buffer->second);
		auto element_size = buffer->GetSuballocationSize();

		auto& buffer_descriptor = buffer_descriptors[buffer_count];
		buffer_descriptor.buffer = Handle(*buffer);
		buffer_descriptor.range = element_size;
		buffer_descriptor.offset = render_device->swapchain.currentFrame * element_size;

		auto& write_descriptor_set = write_descriptor_sets.emplace_back();
		write_descriptor_set.descriptorType = vk::DescriptorType::eUniformBuffer;
		write_descriptor_set.dstSet = dest_set;
		write_descriptor_set.dstBinding = uniform_data.binding;
		write_descriptor_set.descriptorCount = 1u;
		write_descriptor_set.pBufferInfo = &buffer_descriptor;

		buffer->Update(uniform_data.data, uniform_data.size, buffer_descriptor.offset);
		buffer_count++;
	}

	// Process bindless textures
	{
		auto& texture_queue = texture_to_update[render_device->swapchain.currentFrame];

		u64 texture_count{ 0ull };
		while (!texture_queue.empty() && texture_count < k_max_texture_update_per_frame)
		{
			auto texture_id = texture_queue.front();
			auto& texture = textures[texture_id];

			auto& start_element_index = texture_index_remap[texture_id];
			bool is_storage_image = utl::cast<bool>(texture->info.flags & ETextureCreateFlagBits::eStorage);

			auto& mip_level = texture->getSubresource();
			auto& image_descriptor = image_descriptors[texture_count];
			image_descriptor.imageView = mip_level.imageView;
			image_descriptor.sampler = texture->sampler != ~0ull ? sampler_cache[texture->sampler] : default_sampler;
			image_descriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (texture->info.flags & ETextureCreateFlagBits::eSamplable)
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
		auto& logical = render_device->GetLogicalDevice();
		Handle(logical).updateDescriptorSets(utl::cast<u32>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
	}
}

const utl::vector<uint32_t>& VkRenderBackend::GetGlobalTextureRemap()
{
	return texture_index_remap;
}

FAPIHandle VkRenderBackend::CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	// Create texture handle
	FVkTextureCreateInfo vk_texture_ci{};
	vk_texture_ci.format = lookup_vk_format(texture_ci.format);
	vk_texture_ci.flags = texture_ci.flags;
	vk_texture_ci.size = texture_ci.size;
	vk_texture_ci.mipLevels = texture_ci.mipLevels;
	vk_texture_ci.arrayLayers = texture_ci.arrayLayers;
	vk_texture_ci.sampleCount = vk::SampleCountFlagBits::e1;
	vk_texture_ci.addressMode = vk::SamplerAddressMode::eRepeat;
	vk_texture_ci.enableAnisotropy = true;

	if (texture_ci.flags & ETextureCreateFlagBits::eRenderTarget)
	{
		vk_texture_ci.addressMode = vk::SamplerAddressMode::eClampToEdge;
		vk_texture_ci.enableAnisotropy = false;
	}

#ifndef BUILD_WITHOUT_DEBUGGIND
	vk_texture_ci.name = name;
#endif

	auto texture = std::make_unique<FVkTexture>();
	CreateTexture(vk_texture_ci, *texture);

	return FAPIHandle(render_resources->add<FVkTexture>(name.c_str(), std::move(texture)));
}

FAPIHandle VkRenderBackend::CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, FTextureUploadInfo&& upload_info)
{
	auto& upload_data = texture_upload_infos.emplace_back(std::move(upload_info));
	upload_data.textureId = CreateTexture(name, texture_ci);
	return upload_data.textureId;
}

FAPIHandle VkRenderBackend::GetTextureId(const std::string& name)
{
	return FAPIHandle(render_resources->getId<FVkTexture>(name.c_str()));
}

FTextureView VkRenderBackend::GetTextureView(FAPIHandle texture_id)
{
	auto& texture = render_resources->get<FVkTexture>(texture_id.Get());

	FTextureView tex_view{};
	tex_view.width = texture->info.size.width;
	tex_view.height = texture->info.size.height;
	tex_view.depth = texture->info.size.depth;
	tex_view.arrayLayers = texture->info.arrayLayers;
	tex_view.mipLevels = texture->info.mipLevels;

	return tex_view;
}

void VkRenderBackend::DestroyTexture(FAPIHandle resource_id)
{
	resource_destroy_queue.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eTexture });
}

void VkRenderBackend::ReadPixel(FAPIHandle resource_id, u32 x, u32 y, void* data)
{
	static constexpr u64 pixel_size{ 4ull };

	auto& texture = render_resources->get<FVkTexture>(resource_id.Get());

	// Create temporary command buffer
	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = std::format("pick pixel #{}(x:{},y{})", resource_id.Get(), x, y);
#endif

	FVkCommandBuffer command_buffer = CreateCommandBuffer(command_buffer_ci);
	render_device->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0);
	auto& current = command_buffer.get();

	auto builder = BufferBuilder(pixel_size, 1ull, 1ull);
	builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
	builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);

#ifndef BUILD_WITHOUT_DEBUGGIND
	builder.SetDebugName(std::format("pick pixel #{}(x:{},y{})", resource_id.Get(), x, y));
#endif

	auto staging_buffer = builder.BuildUnique(*render_device);

	vk::BufferImageCopy region;
	region.bufferOffset = 0u;
	region.bufferImageHeight = 0u;
	region.bufferRowLength = 0u;

	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0u;
	region.imageSubresource.baseArrayLayer = 0u;
	region.imageSubresource.layerCount = 1u;

	region.imageOffset.x = x;
	region.imageOffset.y = y;
	region.imageExtent.width = 1u;
	region.imageExtent.height = 1u;
	region.imageExtent.depth = 1u;

	current.copyImageToBuffer(texture->image.handle, vk::ImageLayout::eTransferSrcOptimal, Handle(*staging_buffer), 1, &region);

	render_device->SubmitCommandBuffer(command_buffer);
	DestroyCommandBuffer(EQueueTypeFlagBits::eGraphics);
	// TODO: DESTROY COMMAND BUFFER AND DELETE IT

	// Copy data to buffer
	void* mapped_buffer_memory = staging_buffer->Map();
	util::Copy(data, mapped_buffer_memory, pixel_size);
	staging_buffer->Unmap();
}


FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	CVkShaderReflectionBuilder reflection_builder{};

	FVkGraphicsPipelineCreateInfo pipeline_ci{};
	pipeline_ci.primitiveTopology = get_primitive_topology(shader_ci.primitiveTopology);
	pipeline_ci.cullMode = get_cull_mode_flags(shader_ci.cullMode);
	pipeline_ci.frontFace = get_front_face(shader_ci.frontFace);
	pipeline_ci.depthCompareOp = get_compare_op(shader_ci.depthCompareOp);
	pipeline_ci.enableAlphaBlend = shader_ci.alphaBlending;
#ifndef BUILD_WITHOUT_DEBUGGIND
	pipeline_ci.name = name;
#endif

	// Get color attachment formats
	auto& textures = render_resources->data<FVkTexture>();
	for (auto& attachment : shader_ci.colorAttachments)
	{
		FVkTexture* texture{ nullptr };
		if (attachment.backbuffer)
			texture = &backbuffer[render_device->swapchain.currentFrame];
		else
			texture = textures[attachment.source.Get()].get();

		pipeline_ci.colorAttachmentFormats.emplace_back(texture->info.format);
	}

	// Get depth format
	if(shader_ci.depthAttachment.source)
	{
		auto& texture = textures[shader_ci.depthAttachment.source.Get()];
		pipeline_ci.depthAttachmentFormat = texture->info.format;
	}

	// Get stencil format
	if (shader_ci.stencilAttachment.source)
	{
		auto& texture = textures[shader_ci.stencilAttachment.source.Get()];
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
		render_device->CreateShaderModule(shader_module_ci, shader_module);

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

	shader->reflection = render_resources->add<FVkShaderReflectionData>(name.c_str(), std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	auto res = CreateShaderResources(resources_info, *shader);

	pipeline_ci.layout = shader->layout;

	shader->bindPoint = vk::PipelineBindPoint::eGraphics;

	auto& pipeline = shader->handles.emplace_back();
	render_device->CreatePipeline(pipeline_ci, pipeline);
	
	shader->isAlive = true;

	auto shader_unique_name = create_object_unique_name("shader:{:x}:{}", shader->handles.back(), vk::to_string(shader->bindPoint));
	return FAPIHandle(render_resources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader)));
}

FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	CVkShaderReflectionBuilder reflection_builder{};
	utl::vector<vk::PipelineShaderStageCreateInfo> pipeline_stages{};

	// Create shader modules and collect reflection data
	for (u64 idx = 0ull; idx < shader_stages.size(); ++idx)
	{
		auto& code = shader_stages[idx];
		FVkShaderModuleCreateInfo shader_module_ci{};
		shader_module_ci.shaderCode = code.code;
#ifndef BUILD_WITHOUT_DEBUGGIND
		shader_module_ci.name = std::format("{} stage {} #{}", name, vk::to_string(vk::ShaderStageFlagBits::eCompute), idx);
#endif

		auto& shader_module = shader->shaderModules.emplace_back();
		render_device->CreateShaderModule(shader_module_ci, shader_module);

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

	shader->reflection = render_resources->add<FVkShaderReflectionData>(name.c_str(), std::move(reflection));

	FVkShaderResourcesInfo resources_info{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	resources_info.name = name;
#endif
	resources_info.usageCount = shader_ci.usageCount;

	CreateShaderResources(resources_info, *shader);

	FVkComputePipelineCreateInfo pipeline_ci{};
	pipeline_ci.layout = shader->layout;

	for (u64 idx = 0ull; idx < pipeline_stages.size(); ++idx)
	{
		auto& stage = pipeline_stages[idx];
		pipeline_ci.shaderStage = stage;
#ifndef BUILD_WITHOUT_DEBUGGIND
		pipeline_ci.name = std::format("{} #{}", name, idx);
#endif

		auto& pipeline = shader->handles.emplace_back();
		render_device->CreatePipeline(pipeline_ci, pipeline);
	}

	shader->bindPoint = vk::PipelineBindPoint::eCompute;

	std::stringstream ss{};
	ss << name;
	for (const auto& handle : shader->handles)
		ss << ":" << std::hex << reinterpret_cast<u64>(utl::cast<vk::Pipeline::CType>(handle));

	auto shader_unique_name = ss.str();
	return FAPIHandle(render_resources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader)));
}

FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci, utl::vector<FShaderCreationBlockInfo>&& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader = std::make_unique<FVkShader>();
	shader->bindPoint = vk::PipelineBindPoint::eRayTracingKHR;

	std::stringstream ss{};
	ss << name;
	for (const auto& handle : shader->handles)
		ss << ":" << std::hex << reinterpret_cast<u64>(utl::cast<vk::Pipeline::CType>(handle));

	auto shader_unique_name = ss.str();
	return FAPIHandle(render_resources->add<FVkShader>(shader_unique_name.c_str(), std::move(shader)));
}

FShaderReflectionData VkRenderBackend::GetShaderReflection(FAPIHandle shader_view)
{
	auto& shader = render_resources->get<FVkShader>(shader_view.Get());
	auto& shader_reflection = render_resources->get<FVkShaderReflectionData>(shader->reflection);
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
void VkRenderBackend::UpdateShaderResources(const FShaderResourceCommit& shader_resources, EShaderResourcesUpdateFlags flags)
{
	auto& thread_context = per_thread_context[std::this_thread::get_id()];

	auto& shader = render_resources->get<FVkShader>(shader_resources.shaderId.Get());
	auto& shader_reflection = render_resources->get<FVkShaderReflectionData>(shader->reflection);

	auto& textures = render_resources->data<FVkTexture>();
	auto& buffers = render_resources->data<Buffer>();

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
			auto& buffer = buffers[buffer_id];
			auto buffer_element_size = buffer->GetSuballocationSize();

			// Not sure that it is correct, because i don't know how to determine previous buffer
			auto& buffer_descriptor = buffer_descriptors[buffer_count];
			buffer_descriptor.buffer = Handle(*buffer);
			buffer_descriptor.range = uniform_block.size;
			buffer_descriptor.offset = descriptor_index * buffer_element_size;
			buffer_count++;

			auto& current_descriptor_set = shader->descriptorSets[uniform_block.set].handles[descriptor_index];

			auto& write_descriptor_set = write_descriptor_sets[writes_count];
			write_descriptor_set.descriptorType = uniform_block.descriptorType;
			write_descriptor_set.dstBinding = uniform_block.binding;
			write_descriptor_set.dstSet = current_descriptor_set;
			write_descriptor_set.pBufferInfo = &buffer_descriptor;
			write_descriptor_set.descriptorCount = 1;
			writes_count++;

			// Write new data to buffer memory
			buffer->Update(uniform_data.data, uniform_data.size, buffer_descriptor.offset);
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

			auto& buffer = buffers[buffer_id.Get()];

			// Not sure that it is correct, because i don't know how to determine previous buffer
			auto& buffer_descriptor = buffer_descriptors[buffer_count];
			buffer_descriptor.buffer = Handle(*buffer);
			buffer_descriptor.range = buffer->GetSize();
			buffer_descriptor.offset = 0ull;
			buffer_count++;

			auto descriptor_index = shader->descriptorUsage[uniform_block.set];
			auto& current_descriptor_set = shader->descriptorSets[uniform_block.set].handles[descriptor_index];

			auto& write_descriptor_set = write_descriptor_sets[writes_count];
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
			auto& texture = textures[bind_info.textureId.Get()];
			auto& texture_subresource = texture->getSubresource(bind_info.mipLevel);

			auto found_reflection = shader_reflection->uniforms.find(texture_name);
			if (found_reflection == shader_reflection->uniforms.end())
				continue;

			auto& uniform = found_reflection->second;
			updated_descriptor_sets.emplace(uniform.set);

			auto& image_descriptor = image_descriptors[texture_count];
			image_descriptor.imageView = texture_subresource.imageView;
			image_descriptor.sampler = texture->sampler != ~0ull ? sampler_cache[texture->sampler] : default_sampler;
			image_descriptor.imageLayout = texture_subresource.layout;
			texture_count++;

			auto descriptor_index = shader->descriptorUsage[uniform.set];
			auto& current_descriptor_set = shader->descriptorSets[uniform.set].handles[descriptor_index];

			auto& write_descriptor_set = write_descriptor_sets[writes_count];
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
		auto& logical = render_device->GetLogicalDevice();
		Handle(logical).updateDescriptorSets(utl::cast<u32>(writes_count), write_descriptor_sets.data(), 0, nullptr);
	}
}

void VkRenderBackend::FlushConstantRanges(FAPIHandle shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type)
{
	auto& shader = render_resources->get<FVkShader>(shader_id.Get());
	auto& shader_reflection = render_resources->get<FVkShaderReflectionData>(shader->reflection);

	auto& command_buffer = GetLastCommandBuffer(queue_type);
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

void VkRenderBackend::BindShader(FAPIHandle shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& shader = render_resources->get<FVkShader>(shader_id.Get());

	// TODO: make possible to select what pipeline i have to bind now
	current.bindPipeline(shader->bindPoint, shader->handles.back());
}

void VkRenderBackend::BindShaderResources(FAPIHandle shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& shader = render_resources->get<FVkShader>(shader_id.Get());

	BindShaderDescriptors(command_buffer, *shader);
}

void VkRenderBackend::DestroyShader(FAPIHandle resource_id)
{
	resource_destroy_queue.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eShader });
}

FAPIHandle VkRenderBackend::CreateBuffer(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto& physical = render_device->GetPhysicalDevice();
	auto builder = MakeBufferBuilder(buffer_ci, physical.GetProperties());

	auto buffer = builder.BuildUnique(*render_device);

	auto buffer_unique_name = create_object_unique_name("buffer:{:x}:{}", Handle(*buffer), name);
	return FAPIHandle(render_resources->add<Buffer>(buffer_unique_name.c_str(), std::move(buffer)));
}

void VkRenderBackend::UpdateBuffer(const std::string& name, FAPIHandle buffer_id, const FBufferCreateInfo& buffer_ci)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());

	auto& physical = render_device->GetPhysicalDevice();
	auto builder = MakeBufferBuilder(buffer_ci, physical.GetProperties());
	buffer = builder.BuildUnique(*render_device);
}

void VkRenderBackend::UploadDataToBuffer(const FBufferUploadInfo& upload_info)
{
	buffer_upload_infos.emplace_back(upload_info);
}

FBufferView VkRenderBackend::GetBufferView(FAPIHandle buffer_id)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());
	return { buffer->GetSize(), buffer->GetSize() / buffer->GetSuballocationSize(), buffer->GetDeviceAddress()};
}

void VkRenderBackend::BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	utl::vector<vk::Buffer> buffers{};
	for (const auto& buffer_id : binding_info.buffers)
		buffers.emplace_back(Handle(*render_resources->get<Buffer>(buffer_id.Get())));

	current.bindVertexBuffers(binding_info.firstBinding, buffers, binding_info.offsets);
}

void VkRenderBackend::BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	vk::IndexType index_type{ vk::IndexType::eNoneKHR };
	switch (binding_info.indexSize)
	{
	case 4: index_type = vk::IndexType::eUint32; break;
	case 2: index_type = vk::IndexType::eUint16; break;
	case 1: index_type = vk::IndexType::eUint8EXT; break;
	}

	auto& buffer = render_resources->get<Buffer>(binding_info.bufferId.Get());
	current.bindIndexBuffer(Handle(*buffer), binding_info.offset, index_type);
}

void VkRenderBackend::MapBufferMemory(FAPIHandle buffer_id, void*& memory)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());
	memory = buffer->Map();
}

void VkRenderBackend::UnmapBufferMemory(FAPIHandle buffer_id)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());
	buffer->Unmap();
}

void VkRenderBackend::FlushBuffer(FAPIHandle buffer_id, u64 size, u64 offset)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());
	buffer->Flush(offset, size);
}

void VkRenderBackend::DestroyBuffer(FAPIHandle resource_id)
{
	resource_destroy_queue.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eBuffer });
}

void VkRenderBackend::UploadBuffers()
{
	ProcessResourceUpload();
}

void VkRenderBackend::PushG2CBarriers(EQueueTypeFlagBits queue_type, const utl::vector<FAPIHandle>& texture_ids)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& textures = render_resources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures[texture_id.Get()];

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

	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::PushC2CBarrier(EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& textures = render_resources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	auto& barrier = translation_info.memoryTranslationInfos.emplace_back();
	barrier.srcStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.srcAccessMask = vk::AccessFlagBits2::eShaderWrite;
	barrier.dstStageMask = vk::PipelineStageFlagBits2::eComputeShader;
	barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::PushC2GBarrier(EQueueTypeFlagBits queue_type, const utl::vector<FAPIHandle>& texture_ids)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& textures = render_resources->data<FVkTexture>();

	FVkTranslationInfo translation_info{};
	for (auto texture_id : texture_ids)
	{
		auto& texture = textures[texture_id.Get()];
	
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

	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers)
{
	auto& command_buffer = GetLastCommandBuffer(queue);

	auto& textures = render_resources->data<FVkTexture>();
	auto& buffers = render_resources->data<Buffer>();

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
				auto& texture = textures[resource_barrier.resource.handle.Get()];
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
					texture = &backbuffer[render_device->swapchain.currentFrame];
				else
					texture = textures[resource_barrier.resource.handle.Get()].get();

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
				auto& buffer = buffers[resource_barrier.resource.handle.Get()];

				FVkBufferTranslationInfo buffer_translation_info{};
				buffer_translation_info.handle = Handle(*buffer);
				buffer_translation_info.size = buffer->GetSize();
				buffer_translation_info.srcStageMask = get_translation_stage_flags(resource_barrier.srcPassType);
				buffer_translation_info.srcAccessMask = get_translation_access_flags(resource_barrier.srcPassType, src_resource_access);
				buffer_translation_info.dstStageMask = get_translation_stage_flags(resource_barrier.dstPassType);
				buffer_translation_info.dstAccessMask = get_translation_access_flags(resource_barrier.dstPassType, dst_resource_access);
				translation_info.bufferTranslationInfos.emplace_back(buffer_translation_info);
			}
		}
	}

	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::BeginTimestampZone(EQueueTypeFlagBits queue)
{
	//assert(zone_entries[queue] == 0 && "Previous zone was not ended.");
	auto& command_buffer = GetLastCommandBuffer(queue);
	render_device->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eTopOfPipe, timestamp_query);
}

void VkRenderBackend::EndTimestampZone(EQueueTypeFlagBits queue)
{
	auto& command_buffer = GetLastCommandBuffer(queue);
	render_device->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eBottomOfPipe, timestamp_query);
}

void VkRenderBackend::BeginRendering(const FBeginRendering2Info& rendering_info)
{
	auto& command_buffer = GetLastCommandBuffer(rendering_info.queue);

	auto& textures = render_resources->data<FVkTexture>();

	utl::hash_map<u64, vk::ClearValue> clear_values{};
	for (const auto& [handle, clear_value] : rendering_info.clearValues)
	{
		auto& new_value = clear_values[handle.Get()];

		if (handle)
		{
			auto& texture = textures[handle.Get()];

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
				texture = &backbuffer[render_device->swapchain.currentFrame];
			else
				texture = textures[color_attachment_info.handle.Get()].get();

			auto& target_subresource = texture->getSubresource(color_attachment_info.mipLevel);
			attachment_info.imageView = target_subresource.imageView;
			attachment_info.imageLayout = target_subresource.layout;

			attachment_info.resolveMode = vk::ResolveModeFlagBits::eNone;
			attachment_info.loadOp = get_attachment_load_op(color_attachment_info.loadOp);
			attachment_info.storeOp = get_attachment_store_op(color_attachment_info.storeOp);
			attachment_info.clearValue = clear_values[color_attachment_info.handle.Get()];
		}
	}

	// Prepare depth target
	vk::RenderingAttachmentInfoKHR depth_attachment{};
	if (rendering_info.depthAttachment && rendering_info.depthAttachment->handle)
	{
		auto& depth_target = textures[rendering_info.depthAttachment->handle.Get()];
		auto& target_subresource = depth_target->getSubresource(rendering_info.depthAttachment->mipLevel);

		depth_attachment.imageView = target_subresource.imageView;
		depth_attachment.imageLayout = target_subresource.layout;
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = get_attachment_load_op(rendering_info.depthAttachment->loadOp);
		depth_attachment.storeOp = get_attachment_store_op(rendering_info.depthAttachment->storeOp);
		depth_attachment.clearValue = clear_values[rendering_info.depthAttachment->handle.Get()];
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle && rendering_info.stencilAttachment->handle)
	{
		auto& stencil_target = textures[rendering_info.stencilAttachment->handle.Get()];
		auto& target_subresource = stencil_target->getSubresource(rendering_info.stencilAttachment->mipLevel);

		stencil_attachment.imageView = target_subresource.imageView;
		stencil_attachment.imageLayout = target_subresource.layout;
		stencil_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		stencil_attachment.loadOp = get_attachment_load_op(rendering_info.stencilAttachment->loadOp);
		stencil_attachment.storeOp = get_attachment_store_op(rendering_info.stencilAttachment->storeOp);
		stencil_attachment.clearValue = clear_values[rendering_info.stencilAttachment->handle.Get()];
	}

	vk::RenderingInfoKHR begin_rendering_info{};
	begin_rendering_info.renderArea.offset.x = rendering_info.renderArea.offset.x;
	begin_rendering_info.renderArea.offset.y = rendering_info.renderArea.offset.y;
	begin_rendering_info.renderArea.extent.width = rendering_info.renderArea.size.x;
	begin_rendering_info.renderArea.extent.height = rendering_info.renderArea.size.y;
	begin_rendering_info.colorAttachmentCount = utl::cast<u32>(color_attachments.size());
	begin_rendering_info.pColorAttachments = color_attachments.data();
	begin_rendering_info.layerCount = rendering_info.layerCount;

	if (rendering_info.depthAttachment && rendering_info.depthAttachment->handle)
	{
		begin_rendering_info.pDepthAttachment = &depth_attachment;

		auto& depth_target = textures[rendering_info.depthAttachment->handle.Get()];
		if (!is_depth_only_format(depth_target->info.format) && rendering_info.stencilAttachment->handle == rendering_info.depthAttachment->handle)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment->handle && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	render_device->DebugBeginLabel(current_buffer, rendering_info.name, 0x5555ffff);
#endif

	current_buffer.beginRenderingKHR(begin_rendering_info);
}

void VkRenderBackend::EndRendering(EQueueTypeFlagBits queue)
{
	auto& command_buffer = GetLastCommandBuffer(queue);

	auto& current_buffer = command_buffer.get();
	current_buffer.endRenderingKHR();

#ifndef BUILD_WITHOUT_DEBUGGIND
	render_device->EndLabel(current_buffer);
#endif
}

void VkRenderBackend::BeginCompute(const FBeginComputeInfo& compute_info)
{
	auto& command_buffer = GetLastCommandBuffer(compute_info.queue);
	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	render_device->DebugBeginLabel(current_buffer, compute_info.name, 0x55ff55ff);
#endif
}

void VkRenderBackend::EndCompute(EQueueTypeFlagBits queue)
{
	auto& command_buffer = GetLastCommandBuffer(queue);
	auto& current_buffer = command_buffer.get();

#ifndef BUILD_WITHOUT_DEBUGGIND
	render_device->EndLabel(current_buffer);
#endif
}

void VkRenderBackend::ProcessMemoryBarriers(const FVkCommandBuffer& command_buffer, const FVkTranslationInfo& translation_info)
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

void VkRenderBackend::SetViewport(f32 width, f32 height, bool flip_viewport)
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
	auto& command_buffer = GetLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	auto& current = command_buffer.get();

	current.setViewport(0, 1, &viewport);
}

void VkRenderBackend::SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height)
{
	vk::Rect2D scissor_rect{};
	scissor_rect.offset.x = offset_x;
	scissor_rect.offset.y = offset_y;
	scissor_rect.extent.width = width;
	scissor_rect.extent.height = height;

	// Because i can set family only in graphics
	auto& command_buffer = GetLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	auto& current = command_buffer.get();

	current.setScissor(0, 1, &scissor_rect);
}

void VkRenderBackend::SetPrimitiveTopology(EVertexPrimitiveTopology primitivd_topology)
{
	auto& command_buffer = GetLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	auto& current = command_buffer.get();

	current.setPrimitiveTopology(get_primitive_topology(primitivd_topology));
}

// TODO: add indirect Draw support
void VkRenderBackend::Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
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

void VkRenderBackend::Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();
	//current.drawMeshTasksIndirectEXT()

	auto& indirect_command_buffer = render_resources->get<Buffer>(draw_info.bufferId.Get());
	auto& indirect_count_buffer = render_resources->get<Buffer>(draw_info.countBufferId.Get());
	current.drawIndexedIndirectCount(Handle(*indirect_command_buffer), draw_info.offset, Handle(*indirect_count_buffer), draw_info.countOffset, draw_info.maxDrawCount, draw_info.stride);
	//current.drawIndexedIndirect(indirect_command_buffer->handle, draw_info.offset, draw_info.count, draw_info.stride);
}

void VkRenderBackend::Dispatch(EQueueTypeFlagBits queue_type, const FDispatch2Info& dispatch_info, u64 subgroup_index)
{
	vk::DispatchIndirectCommand command{};
	command.x = dispatch_info.globalSizes.x;
	command.y = dispatch_info.globalSizes.y;
	command.z = dispatch_info.globalSizes.z;

	if (dispatch_info.divideByLocals)
	{
		auto& shader = render_resources->get<FVkShader>(dispatch_info.shaderHandle.Get());
		auto& reflection = render_resources->get<FVkShaderReflectionData>(shader->reflection);

		auto& localSizes = reflection->localSizes[subgroup_index];
		command.x = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.x) / utl::cast<f32>(localSizes[0]))), 1u);
		command.y = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.y) / utl::cast<f32>(localSizes[1]))), 1u);
		command.z = std::max(utl::cast<u32>(std::ceil(utl::cast<f32>(command.z) / utl::cast<f32>(localSizes[2]))), 1u);
	}

	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();
	current.dispatch(command.x, command.y, command.z);
}

void VkRenderBackend::Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	auto& argument_buffer = render_resources->get<Buffer>(dispatch_info.argumentBuffer.Get());

	if (dispatch_info.countBuffer)
	{
		auto& argument_buffer = render_resources->get<Buffer>(dispatch_info.argumentBuffer.Get());
		// Dispatch indirect count is not supported
	}
	else
		current.dispatchIndirect(Handle(*argument_buffer), dispatch_info.argumentOffset);
}

FDeviceLimitations VkRenderBackend::GetDeviceLimitations()
{
	FDeviceLimitations limits{};

	return limits;
}

const FRenderFrameStats& VkRenderBackend::GetRenderFrameStats() const
{
	return render_stats;
}

EAdapterVendor VkRenderBackend::GetDeviceVendor() const
{
	auto& physical = render_device->GetPhysicalDevice();

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

FAPIHandle VkRenderBackend::GetTextureResource(const char* name)
{
	return FAPIHandle(render_resources->getId<FVkTexture>(name));
}

FAPIHandle VkRenderBackend::GetShaderResource(const char* name)
{
	return FAPIHandle(render_resources->getId<FVkShader>(name));
}

FPoint2D<u32> VkRenderBackend::GetRenderResolution() const
{
	FPoint2D<u32> resolution{};
	resolution.x = render_device->swapchain.extent.width;
	resolution.y = render_device->swapchain.extent.height;

	return resolution;
}

FRect2D<f32> VkRenderBackend::GetRenderRectFromRT(FAPIHandle texture_id)
{
	auto& texture = render_resources->get<FVkTexture>(texture_id.Get());

	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(texture->info.size.width);
	rect.size.y = utl::cast<f32>(texture->info.size.height);

	return rect;
}

FRect2D<f32> VkRenderBackend::GetScreenRenderRect()
{
	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(render_device->swapchain.extent.width);
	rect.size.y = utl::cast<f32>(render_device->swapchain.extent.height);
	return rect;
}

// TODO: update only targets, that have same size with swapchain
void VkRenderBackend::UpdateSurface(i32 new_width, i32 new_height)
{
	if (this->surface_width != new_width || this->m_iSurfaceHeight != new_height)
	{
		this->surface_width = new_width;
		this->m_iSurfaceHeight = new_height;

		auto& logical_device = render_device->GetLogicalDevice();
		Handle(logical_device).waitIdle();

		render_device->CleanupSwapchain();
		render_device->CreateSwapchain(new_width, new_height);
		render_device->swapchain.currentFrame = 0u;

		auto& textures = render_resources->data<FVkTexture>();

		// Updating render targets
		for (u64 texture_id = 0ull; texture_id < textures.size(); ++texture_id)
		{
			bool is_storage{ false };
			u64 swapchainFrameIndex{ 0ull };

			auto& texture = textures[texture_id];

			if (!(texture->info.flags & ETextureCreateFlagBits::eRenderTarget) || (texture->info.size.policy == ERenderTargetSizePolicy::eFixed))
				continue;

			for (auto& mip : texture->mips)
				render_device->DestroyImageView(mip.imageView);
			texture->mips.clear();

			render_device->DestroyImageView(texture->base.imageView);
			render_device->DestroyImage(texture->image);

			FVkTextureCreateInfo texture_ci{};
			texture_ci.format = texture->info.format;
			texture_ci.flags = texture->info.flags;
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
				texture_ci.size.width = utl::cast<u32>(utl::cast<f32>(new_width) * texture_ci.size.scale);
				texture_ci.size.height = utl::cast<u32>(utl::cast<f32>(new_height)* texture_ci.size.scale);
			}

			check_api_result(CreateTexture(texture_ci, *texture));
			
			is_storage = utl::cast<bool>(texture_ci.flags & ETextureCreateFlagBits::eStorage);

			//if (is_storage)
			//	texture_layout_change_infos.emplace_back(texture_id);
		}

		// Update backbuffer resources
		UpdateBackbuffer();
	}
}

FVkCommandBuffer& VkRenderBackend::CreateCommandBuffer(const FVkCommandBufferCreateInfo& command_buffer_ci, std::thread::id thread_id)
{
	auto& thread_context = per_thread_context[thread_id];
	auto& per_queue = thread_context.perQueue[command_buffer_ci.mask];

	if (!per_queue.commandPool.handle)
	{
		FVkCommandPoolCreateInfo pool_ci{};
		pool_ci.mask = command_buffer_ci.mask;

		check_api_result(render_device->CreateCommandPool(pool_ci, per_queue.commandPool));
	}
	
	auto& command_buffer = per_queue.commandBuffers.emplace_back();
	check_api_result(render_device->CreateCommandBuffer(command_buffer_ci, per_queue.commandPool, command_buffer));

	return command_buffer;
}

FVkCommandBuffer& VkRenderBackend::GetLastCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = per_thread_context[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers.back();
}

FVkCommandBuffer& VkRenderBackend::GetCommandBuffer(EQueueTypeFlagBits queue_type, u64 index, std::thread::id thread_id)
{
	auto& thread_context = per_thread_context[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers[index];
}

utl::vector<FVkCommandBuffer>& VkRenderBackend::GetCommandBuffers(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = per_thread_context[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	return per_queue.commandBuffers;
}

void VkRenderBackend::DestroyCommandBuffer(EQueueTypeFlagBits queue_type, std::thread::id thread_id)
{
	auto& thread_context = per_thread_context[thread_id];
	auto& per_queue = thread_context.perQueue[queue_type];

	auto& command_buffer = per_queue.commandBuffers.back();
	render_device->DestroyCommandBuffer(per_queue.commandPool, command_buffer);
	per_queue.commandBuffers.pop_back();
}

const std::unique_ptr<RenderDevice>& VkRenderBackend::getDevice() const
{
	return render_device;
}

void VkRenderBackend::BeginFrame()
{
	u32 image_index{ 0u };

	ProcessResourceBarriers();
	ProcessResourceUpload();
	// TODO: sync queues here

	vk::Result res{};
	try { res = render_device->AcquireNextImage(image_index); }
	catch (vk::OutOfDateKHRError error) { res = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("Failed to acquire swap chain image!"); }

	if (res == vk::Result::eErrorOutOfDateKHR)
		UpdateSurface(surface_width, m_iSurfaceHeight);
	else if (res == vk::Result::eSuboptimalKHR)
		log_error("Failed to acquire swap chain image!");

	if (res != vk::Result::eSuccess)
		return;

	is_frame_started = true;

	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = GetLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	render_device->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eRenderPassContinue, image_index);
	render_device->ResetTimestampQuery(timestamp_query);
	//render_device->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eTopOfPipe, timestamp_query);

	// Update bindless textures
	auto& textures = render_resources->data<FVkTexture>();

	if (texture_index_remap.empty())
	{
		// Resizing remap vector
		texture_index_remap.resize(textures.size());

		u64 image_offset{ 0ull };
		for (u64 texture_index = 0ull; texture_index < textures.size(); ++texture_index)
		{
			auto& texture = textures[texture_index];
			if (texture->info.flags & ETextureCreateFlagBits::eRenderTarget)
				continue;

			texture_index_remap[texture_index] = image_offset++;

			for (u32 frame_index = 0u; frame_index < render_device->swapchain.framesInFlight; ++frame_index)
				texture_to_update[frame_index].push(texture_index);
		}
	}
}

void VkRenderBackend::EndFrame()
{
	// TODO: here i should run only main command buffer. But for now i getting the last one
	auto& command_buffer = GetLastCommandBuffer(EQueueTypeFlagBits::eGraphics);
	//render_device->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eBottomOfPipe, timestamp_query);

	render_device->EndCommandBuffer(command_buffer);
	is_frame_started = false;

	vk::Result presentResult{};
	try { presentResult = render_device->Present(command_buffer); }
	catch (vk::OutOfDateKHRError err) { presentResult = vk::Result::eErrorOutOfDateKHR; }
	catch (vk::SystemError err) { log_error("failed to present swap chain image!"); }

	utl::vector<f32> results{};
	auto timestamp_result = render_device->GetTimestampQueryResults(timestamp_query, render_stats.timestampResults);
	if (!timestamp_result.fault)
	{
		render_stats.gpuFrameTime = 0.f;
		for (const auto& time : render_stats.timestampResults)
			render_stats.gpuFrameTime += time;
	}

	// Get gpu memory usage info
	render_device->GetOccupedDeviceMemory(vk::MemoryHeapFlagBits::eDeviceLocal, render_stats.totalGPUMemory, render_stats.usedGPUMemory);

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		UpdateSurface(surface_width, m_iSurfaceHeight);
	utl::hash_map<std::string, std::string> kk;
	
	// Destroy unused contexts
	for (auto it = per_thread_context.begin(); it != per_thread_context.end();)
	{
		if (it->second.use_count() < 1)
		{
			DestroyThreadContext(it->second);
			it = per_thread_context.erase(it);
			continue;
		}
		++it;
	}

	// Try to free resources
	if (utilization_counter % render_device->swapchain.framesInFlight == 0)
		ProcessResourceUtilization();

	// Frame finished. Textures in now again in initial layout
	// TODO: i don't need to set layout back to undefined for one time targets or for periodical targets
	auto& textures = render_resources->data<FVkTexture>();
	for (auto& texture : textures)
	{
		if ((texture->info.flags & ETextureCreateFlagBits::eRenderTarget) && !(texture->info.flags & ETextureCreateFlagBits::eStorage))
		{
			// Reset layout for all mip level
			for(auto& mip : texture->mips)
				mip.layout = vk::ImageLayout::eUndefined;

			// Also reset view that stores all other levels
			texture->base.layout = vk::ImageLayout::eUndefined;
		}
	}

	// Also for backbuffer
	for (auto& current : backbuffer)
	{
		for (auto& mip : current.mips)
			mip.layout = vk::ImageLayout::eUndefined;

		current.base.layout = vk::ImageLayout::eUndefined;
	}

	utilization_counter++;
}

bool VkRenderBackend::IsSwapChainFinished()
{
	return (utilization_counter % render_device->swapchain.framesInFlight == 0) && utilization_counter != 0ull;
}

void VkRenderBackend::BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	render_device->DebugBeginLabel(current, label, color);
}

void VkRenderBackend::InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	render_device->DebugInsertLabel(current, label, color);
}

void VkRenderBackend::EndDebugLabel(EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = GetLastCommandBuffer(queue_type);
	auto& current = command_buffer.get();

	render_device->EndLabel(current);
}

void VkRenderBackend::DestroyThreadContext(FVkThreadContext& context)
{
	// Destroy command buffers and pools
	for (auto& [queue_id, queue_resources] : context.perQueue)
	{
		for (auto& command_buffer : queue_resources.commandBuffers)
			render_device->DestroyCommandBuffer(queue_resources.commandPool, command_buffer);

		render_device->DestroyCommandPool(queue_resources.commandPool);
	}

	// Destroy descriptor sets
	for (auto& [id, descriptor_set] : context.descriptorSets)
		render_device->DestroyDescriptorSet(context.descriptorPool, descriptor_set);

	// Destroy descriptor pool
	render_device->DestroyDescriptorPool(context.descriptorPool);
}

void VkRenderBackend::ProcessResourceBarriers()
{
	// TODO: add mip level and array layer support
	if (texture_layout_change_infos.empty())
		return;

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1u;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "resource_barriers";
#endif

	auto& textures = render_resources->data<FVkTexture>();

	auto& command_buffer = CreateCommandBuffer(command_buffer_ci);

	render_device->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	FVkTranslationInfo translation_info{};
	for (const auto& layout_info : texture_layout_change_infos)
	{
		auto& texture = textures[layout_info.textureId];
		auto& target_subresource = texture->getSubresource();

		if (texture->info.flags & ETextureCreateFlagBits::eStorage)
		{
			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
			target_subresource.layout = texture_translation_info.newLayout = vk::ImageLayout::eGeneral;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);
		}
	}

	ProcessMemoryBarriers(command_buffer, translation_info);

	render_device->EndCommandBuffer(command_buffer);
	render_device->SubmitCommandBuffer(command_buffer);

	DestroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

	texture_layout_change_infos.clear();
}

void VkRenderBackend::ProcessResourceUpload()
{
	if (texture_upload_infos.empty() && buffer_upload_infos.empty())
		return;

	FVkCommandBufferCreateInfo command_buffer_ci{};
	command_buffer_ci.count = 1u;
	command_buffer_ci.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_ci.mask = EQueueTypeFlagBits::eGraphics;
#ifndef BUILD_WITHOUT_DEBUGGIND
	command_buffer_ci.name = "resource_upload";
#endif

	auto& textures = render_resources->data<FVkTexture>();

	auto& command_buffer = CreateCommandBuffer(command_buffer_ci);

	render_device->BeginCommandBuffer(command_buffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 0u);

	utl::vector<std::unique_ptr<Buffer>> staging_buffers{};

	// Upload textures
	{
		// Prepare staging buffers
		
		for (auto& texture_upload : texture_upload_infos)
		{
			auto builder = BufferBuilder(texture_upload.data.size(), 1ull, 1ull);
			builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
			builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);

#ifndef BUILD_WITHOUT_DEBUGGIND
			builder.SetDebugName(std::format("texture upload #{}", texture_upload.textureId.Get()));
#endif

			auto& staging_buffer = staging_buffers.emplace_back(builder.BuildUnique(*render_device));

			// Copy data to buffer
			staging_buffer->Update(texture_upload.data);

			texture_upload.data.clear();
		}

		// Translate all images to transfer dst stage
		utl::vector<FAPIHandle> generate_mip_textures{};
		FVkTranslationInfo translation_info{};
		for (const auto& texture_upload : texture_upload_infos)
		{
			auto& texture = textures[texture_upload.textureId.Get()];
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
		ProcessMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();

		// Copy buffer to image
		for (u64 idx = 0ull; idx < texture_upload_infos.size(); ++idx)
		{
			auto& texture_upload = texture_upload_infos[idx];
			auto& texture = textures[texture_upload.textureId.Get()];
			auto& staging_buffer = staging_buffers[idx];

			auto aspect_mask = get_texture_aspect_flags(*texture);

			utl::vector<vk::BufferImageCopy2KHR> copy_regions{};
			if (!texture_upload.mipLayerOffsets.empty())
			{
				for (u32 layer = 0u; layer < texture->totalLayers; ++layer)
				{
					auto& layer_offsets = texture_upload.mipLayerOffsets[layer];
					for (u32 mip = 0u; mip < texture->info.mipLevels; ++mip)
					{
						u64 offset = layer_offsets[mip];
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

			auto& target_subresource = texture->getSubresource();
			staging_buffer->CopyTo(command_buffer.get(), texture->image.handle, target_subresource.layout, copy_regions);
		}

		auto& current_command_buffer = command_buffer.get();
		for (auto& texture_id : generate_mip_textures)
		{
			auto& texture = textures[texture_id.Get()];
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
				
				ProcessImageLayoutTranslation(command_buffer, image_translation_info);

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
				ProcessImageLayoutTranslation(command_buffer, image_translation_info);

				if (mip_width > 1u)
					mip_width /= 2u;
				if (mip_heigth > 1u)
					mip_heigth /= 2u;
			}
		}

		// Translate images to shader read only layout
		for (const auto& texture_upload : texture_upload_infos)
		{
			auto& texture = textures[texture_upload.textureId.Get()];
			auto& target_subresource = texture->getSubresource();

			auto& image_translation = translation_info.imageTranslationInfos.emplace_back();

			FVkTextureTranslationInfo texture_translation_info{};
			texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			target_subresource.layout = texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			image_translation = get_texture_layout_translation_info(*texture, texture_translation_info);
		}

		ProcessMemoryBarriers(command_buffer, translation_info);
		translation_info.imageTranslationInfos.clear();
	}

	// Upload buffers
	for(const auto& upload_info : buffer_upload_infos)
	{
		auto& physical = render_device->GetPhysicalDevice();
		utl::vector<vk::BufferCopy2KHR> copy_regions(upload_info.regions.size());

		auto& dst_buffer = render_resources->get<Buffer>(upload_info.bufferId.Get());
		
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
		auto builder = BufferBuilder(bytes_count, 1ull, 1ull);
		builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
		builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);

#ifndef BUILD_WITHOUT_DEBUGGIND
		builder.SetDebugName(std::format("staging for #{}", upload_info.bufferId.Get()));
#endif

		auto& src_buffer = staging_buffers.emplace_back(builder.BuildUnique(*render_device));

		// Copy data to staging buffer
		void* mapped_memory = src_buffer->Map();

		// Copy upload data to buffer
		src_buffer->Update(upload_info.data);

		auto& current = command_buffer.get();
		src_buffer->CopyTo(current, *dst_buffer, copy_regions);
	}

	render_device->EndCommandBuffer(command_buffer);

	render_device->SubmitCommandBuffer(command_buffer);

	// TODO: submit command buffer. But with pushing semaphores for synchronize with main command buffer
	DestroyCommandBuffer(EQueueTypeFlagBits::eGraphics);

	// destroy staging buffers
	staging_buffers.clear();

	texture_upload_infos.clear();
	buffer_upload_infos.clear();
}

void VkRenderBackend::ProcessResourceUtilization()
{
	while (!resource_destroy_queue.empty())
	{
		auto destroy_resource = resource_destroy_queue.front();

		if (destroy_resource.mask & EResourceFlagBits::eTexture)
		{
			auto& texture = render_resources->get<FVkTexture>(destroy_resource.id.Get());
			DestroyTexture(*texture);
			render_resources->remove<FVkTexture>(destroy_resource.id.Get());
		}
		else if (destroy_resource.mask & EResourceFlagBits::eShader)
		{
			auto& shader = render_resources->get<FVkShader>(destroy_resource.id.Get());
			DestroyShader(*shader);
			render_resources->remove<FVkShader>(destroy_resource.id.Get());
		}
		else if (destroy_resource.mask & EResourceFlagBits::eBuffer)
		{
			auto& buffer = render_resources->get<Buffer>(destroy_resource.id.Get());
			buffer.reset();
			render_resources->remove<Buffer>(destroy_resource.id.Get());
		}

		resource_destroy_queue.pop();
	}
}

void VkRenderBackend::UpdateBackbuffer()
{
	auto& swapchain = render_device->swapchain;

	backbuffer.clear();
	backbuffer.resize(swapchain.framesInFlight);

	for (u64 frame = 0ull; frame < backbuffer.size(); ++frame)
	{
		auto& current = backbuffer[frame];
		current.isAlive = true;
		current.name = "backbuffer_image";
		current.image.handle = swapchain.images[frame];

		auto& mip = current.mips.emplace_back();
		mip.imageView = current.base.imageView = swapchain.imageViews[frame];

		current.info.flags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eRenderTarget;
		current.info.format = swapchain.format;
		current.info.size.width = swapchain.extent.width;
		current.info.size.height = swapchain.extent.height;
	}
}

FRenderApiErrorInfo VkRenderBackend::CreateTexture(const FVkTextureCreateInfo& texture_ci, FVkTexture& texture)
{
	texture.info.format = texture_ci.format;
	texture.info.flags = texture_ci.flags;
	texture.info.size = texture_ci.size;
	texture.info.mipLevels = texture_ci.mipLevels;
	texture.info.arrayLayers = texture_ci.arrayLayers;
	
	FVkImageCreateInfo image_ci{};
	FVkImageViewCreateInfo image_view_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	texture.name = image_ci.name = texture_ci.name;
#endif

	if (texture_ci.size.depth > 1 || (texture_ci.flags & ETextureCreateFlagBits::e3D))
		image_ci.ci.imageType = vk::ImageType::e3D;
	else
	{
		if (texture_ci.flags & ETextureCreateFlagBits::e1D)
			image_ci.ci.imageType = vk::ImageType::e1D;
		else if (texture_ci.flags & ETextureCreateFlagBits::e2D)
			image_ci.ci.imageType = vk::ImageType::e2D;
		else if (texture_ci.flags & ETextureCreateFlagBits::e3D)
			image_ci.ci.imageType = vk::ImageType::e3D;
		else
			image_ci.ci.imageType = vk::ImageType::e2D;
	}

	image_ci.ci.extent = vk::Extent3D{ texture_ci.size.width, texture_ci.size.height, texture_ci.size.depth };
	image_ci.ci.mipLevels = texture_ci.mipLevels;

	image_ci.ci.arrayLayers = texture_ci.arrayLayers;
	if ((texture_ci.flags & ETextureCreateFlagBits::eArray) && (texture_ci.flags & ETextureCreateFlagBits::eCubemap))
	{
		image_ci.ci.arrayLayers *= 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCubeArray;
	}
	else if (texture_ci.flags & ETextureCreateFlagBits::eArray)
		image_view_ci.ci.viewType = vk::ImageViewType::e2DArray;
	else if (texture_ci.flags & ETextureCreateFlagBits::eCubemap)
	{
		image_ci.ci.arrayLayers = 6u;
		image_view_ci.ci.viewType = vk::ImageViewType::eCube;
	}
	else if (texture_ci.flags & ETextureCreateFlagBits::e3D)
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
	image_ci.ci.flags = (texture_ci.flags & ETextureCreateFlagBits::eCubemap) ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

	RETURN_WHEN_FAULT(render_device->CreateImage(image_ci, texture.image));

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

	RETURN_WHEN_FAULT(render_device->CreateImageView(image_view_ci, texture.base.imageView));
	
	texture.mips.resize(texture_ci.mipLevels);
	for (u32 mip_index = 0u; mip_index < texture_ci.mipLevels; ++mip_index)
	{
		auto& current_mip = texture.mips[mip_index];
		image_view_ci.ci.subresourceRange.baseMipLevel = current_mip.baseMipLevel = mip_index;
		image_view_ci.ci.subresourceRange.levelCount = current_mip.mipLevelCount = 1u;

#ifndef BUILD_WITHOUT_DEBUGGIND
		image_view_ci.name = std::format("{}[{}]", texture_ci.name, mip_index);
#endif
		RETURN_WHEN_FAULT(render_device->CreateImageView(image_view_ci, current_mip.imageView));
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

		texture.sampler = GetSampler(sampler_ci);
	}

	texture.isAlive = true;

	return make_api_error(vk::ObjectType::eSampler, vk::Result::eSuccess);
}

void VkRenderBackend::DestroyTexture(FVkTexture& texture)
{	
	for(auto& mip : texture.mips)
		render_device->DestroyImageView(mip.imageView);
	texture.mips.clear();

	render_device->DestroyImageView(texture.base.imageView);
	render_device->DestroyImage(texture.image);

	texture.isAlive = false;
}

u64 VkRenderBackend::GetSampler(const FVkSamplerCreateInfo& create_info)
{
	u64 key = calculate_sampler_hash(create_info);

	auto& sampler = sampler_cache[key];

	if (!sampler)
		render_device->CreateSampler(create_info, sampler);

	return key;
}

FRenderApiErrorInfo VkRenderBackend::CreateShaderResources(const FVkShaderResourcesInfo& resources_info, FVkShader& shader)
{
	auto& reflection = render_resources->get<FVkShaderReflectionData>(shader.reflection);

	// TODO: generate better object names. Current names can overlap

	// Remember maximum shader usage count
	shader.maximumDescriptorSlots = render_device->swapchain.framesInFlight * resources_info.usageCount;

	// Create descriptor set layouts
	for (const auto& [set, layout_bindings] : reflection->descriptorSetLayoutBindings)
	{
		FVkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
		descriptor_set_layout_ci.name = std::format("{}: set #{}", resources_info.name, set);
#endif
		descriptor_set_layout_ci.layoutBindings = layout_bindings; // TODO: may be send pointer to vector here?

		auto& descriptor_set_Layout = shader.descriptorSetLayouts[set];
		RETURN_WHEN_FAULT(render_device->CreateDescriptorSetLayout(descriptor_set_layout_ci, descriptor_set_Layout));
	}

	// Create descriptor pool
	FVkDescriptorPoolCreateInfo pool_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	pool_ci.name = resources_info.name;
#endif
	pool_ci.poolSizes = reflection->descriptorPoolSizes;
	pool_ci.usageCount = resources_info.usageCount * render_device->swapchain.framesInFlight;
	RETURN_WHEN_FAULT(render_device->CreateDescriptorPool(pool_ci, shader.descriptorPool));

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
		RETURN_WHEN_FAULT(render_device->CreateDescriptorSet(descriptor_set_ci, descriptor_set));
	}

	// Minimal buffer offset
	auto& physical = render_device->GetPhysicalDevice();
	auto minOffsetAllignment = std::lcm(physical.GetProperties().limits.minUniformBufferOffsetAlignment, physical.GetProperties().limits.nonCoherentAtomSize);

	// Create uniform buffers
	for (const auto& [id, uniform_block] : reflection->uniformBlocks)
	{
		// Storage buffers is binding from outside
		if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
			continue;

		auto builder = BufferBuilder(uniform_block.size, shader.maximumDescriptorSlots, minOffsetAllignment);
		builder.SetVmaUsage(vma::MemoryUsage::eCpuToGpu);

#ifndef BUILD_WITHOUT_DEBUGGIND
		auto* debug_name = debug_mapping::find(id);
		if (debug_name)
			builder.SetDebugName(std::format("{}:{}", resources_info.name, debug_name));
		else
			builder.SetDebugName(std::format("{}:{:x}", resources_info.name, id));
#endif

		if (uniform_block.descriptorType == vk::DescriptorType::eUniformBuffer)
			builder.SetUsage(vk::BufferUsageFlagBits::eUniformBuffer);
		else if (uniform_block.descriptorType == vk::DescriptorType::eStorageBuffer)
			builder.SetUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer);

		auto buffer = builder.BuildUnique(*render_device);

		auto buffer_name = create_object_unique_name("buffer:{:x}:{}", Handle(*buffer), id);
		auto buffer_id = render_resources->add<Buffer>(buffer_name.c_str(), std::move(buffer));
		shader.uniformBuffers.emplace(id, buffer_id);
	}

	// Create pipeline layout
	FVkPipelineLayoutCreateInfo layout_ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
	layout_ci.name = resources_info.name;
#endif

	// NOTE: adding global descriptor set layout
	layout_ci.descriptorSetLayouts.emplace_back(global_descriptor.deecriptorSetLayout);

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

	return render_device->CreatePipelineLayout(layout_ci, shader.layout);
}

// Destroy shader object
void VkRenderBackend::DestroyShader(FVkShader& shader)
{
	for (auto& pipeline : shader.handles)
		render_device->DestroyPipeline(pipeline);

	for (auto& shader_module : shader.shaderModules)
		render_device->DestroyShaderModule(shader_module);

	for (auto& [set, descriptor_set] : shader.descriptorSets)
		render_device->DestroyDescriptorSet(shader.descriptorPool, descriptor_set);

	for (auto& [id, buffer_id] : shader.uniformBuffers)
	{
		auto& buffer = render_resources->get<Buffer>(buffer_id);
		buffer.reset();
		render_resources->remove<Buffer>(buffer_id);
	}

	render_device->DestroyDescriptorPool(shader.descriptorPool);

	render_device->DestroyPipelineLayout(shader.layout);

	for (auto& [set, descriptor_set_layout] : shader.descriptorSetLayouts)
		render_device->DestroyDescriptorSetLayout(descriptor_set_layout);

	shader.isAlive = false;
}

void VkRenderBackend::BindShaderDescriptors(const FVkCommandBuffer& command_buffer, const FVkShader& shader)
{
	utl::vector<vk::DescriptorSet> bindDescriptorSets{};

	// NOTE: bind global descriptor set
	bindDescriptorSets.emplace_back(global_descriptor.descriptorSet.handles[render_device->swapchain.currentFrame]);

	// NOTE: shader.descriptorUsage contains next index of descriptor set to use. 
	// Here i collecting all shader descriptor sets, that was changed in past
	for (auto& [set, usage_index] : shader.descriptorUsage)
	{
		auto actual_descriptor_usage = usage_index == 0u ? shader.maximumDescriptorSlots - 1u : usage_index - 1u;
		auto found_descriptor_sets = shader.descriptorSets.find(set);
		if (found_descriptor_sets != shader.descriptorSets.end())
		{
			auto& descriptor_set = found_descriptor_sets->second;
			auto& target_descriptor_set = descriptor_set.handles[actual_descriptor_usage];
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

void VkRenderBackend::ProcessImageLayoutTranslation(const FVkCommandBuffer& command_buffer, const FVkImageTranslationInfo& image_translation_info)
{
	FVkTranslationInfo translation_info{};
	translation_info.imageTranslationInfos.emplace_back(image_translation_info);
	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::TextureLayoutTranslation(const FVkCommandBuffer& command_buffer, FVkTexture& texture, const FVkTextureTranslationInfo& translation_info)
{
	auto image_layout_translation = get_texture_layout_translation_info(texture, translation_info);
	ProcessImageLayoutTranslation(command_buffer, image_layout_translation);
}
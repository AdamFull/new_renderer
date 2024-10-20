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
#include <engine/render/render_api/vulkan/Sampler.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Texture.h>
#include <engine/render/render_api/vulkan/CommandPool.h>
#include <engine/render/render_api/vulkan/Swapchain.h>
#include <engine/render/render_api/vulkan/RenderContext.h>
#include <engine/render/render_api/vulkan/UploadContext.h>
#include <engine/render/render_api/vulkan/DescriptorPool.h>
#include <engine/render/render_api/vulkan/Shader.h>

using namespace engine;
using namespace engine::render;
using namespace engine::render::vulkan;

static const u32 k_max_bindless_buffers{ 256u };

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
		memory_usage_flags = vma::MemoryUsage::eGpuOnly;
	else if (buffer_ci.flags & EBufferUsageFlagBits::eCpuToGpu)
		memory_usage_flags = vma::MemoryUsage::eCpuToGpu;
	else
		memory_usage_flags = vma::MemoryUsage::eCpuOnly;

	// Add buffer alignment
	// std::lcm(minimal_alignment, physical.properties.limits.nonCoherentAtomSize);
	auto builder = BufferBuilder(buffer_ci.size, buffer_ci.count, minimal_alignment);
	builder.SetVmaUsage(memory_usage_flags);
	builder.SetUsage(usage_flags);
	builder.SetSharingMode(vk::SharingMode::eConcurrent);

	return builder;
}

VkRenderBackend::~VkRenderBackend()
{
	auto& logical_device = render_device->GetLogicalDevice();
	Handle(logical_device).waitIdle();

	command_pool.reset();

	// Destroy all textures
	auto& textures = render_resources->data<Texture>();
	textures.clear();

	// Destroy all shaders
	auto& shaders = render_resources->data<Shader>();
	shaders.clear();

	// Destroy all buffers
	auto& buffers = render_resources->data<Buffer>();
	buffers.clear();

	default_sampler.reset();
	sampler_cache.clear();
}

void VkRenderBackend::Create(const FRenderContextCreateInfo& context_ci)
{
	render_device = std::make_unique<RenderDevice>();
	render_device->Create(context_ci);

	render_context = std::make_unique<RenderContext>(*render_device, context_ci.window);
	upload_context = std::make_unique<UploadContext>(*render_device);

	render_resources = std::make_unique<vk_resource_container_t>();

	auto command_pool_builder = CommandPoolBuilder();
	command_pool_builder.SetDebugName("Graphics");
	command_pool_builder.SetResetMode(CommandBuffer::ResetMode::ResetIndividually);
	// TODO: Add queue selection in builder
	command_pool_builder.SetQueueFamily(render_device->GetLogicalDevice().GetQueueByFlags(vk::QueueFlagBits::eGraphics, 0).GetFamilyIndex());

	command_pool = command_pool_builder.BuildUnique(*render_device);

	auto res = render_device->CreateTimestampQuery(256, timestamp_query);

	// Create default sampler
	auto sampler_builder = SamplerBuilder();
	sampler_builder.SetAddressMode(vk::SamplerAddressMode::eClampToEdge);
	sampler_builder.SetMinFilter(vk::Filter::eLinear);
	sampler_builder.SetMagFilter(vk::Filter::eLinear);
	sampler_builder.SetDebugName("default");
	
	default_sampler = sampler_builder.BuildUnique(*render_device);
}

void VkRenderBackend::CreateGlobalDescriptor(const FGlobalUniformCreateInfo& uniform_ci)
{
	render_context->CreateGlobalDescriptors(uniform_ci);
}

void VkRenderBackend::UpdateGlobalDescriptorData(const FGlobalUniformUpdateInfo& data_view)
{
	auto& dest_set = render_context->GetCurrentGlobalDescriptorSet();

	std::array<vk::DescriptorImageInfo, k_max_texture_update_per_frame> image_descriptors{};
	std::array<vk::DescriptorBufferInfo, k_max_buffer_update_per_frame> buffer_descriptors{};
	utl::vector<vk::WriteDescriptorSet> write_descriptor_sets{};

	auto& textures = render_resources->data<Texture>();

	// Bind and update global uniform
	u64 buffer_count{ 0ull };
	for (const auto& [uniform_name, uniform_data] : data_view.uniforms)
	{
		auto* buffer = render_context->GetGlobalUniformBuffer(uniform_name);
		if (!buffer)
			continue;

		auto element_size = buffer->GetSuballocationSize();

		auto& buffer_descriptor = buffer_descriptors[buffer_count];
		buffer_descriptor.buffer = Handle(*buffer);
		buffer_descriptor.range = element_size;
		buffer_descriptor.offset = render_context->GetCurrentFrameIndex() * element_size;

		auto& write_descriptor_set = write_descriptor_sets.emplace_back();
		write_descriptor_set.descriptorType = vk::DescriptorType::eUniformBuffer;
		write_descriptor_set.dstSet = dest_set.GetHandle();
		write_descriptor_set.dstBinding = uniform_data.binding;
		write_descriptor_set.descriptorCount = 1u;
		write_descriptor_set.pBufferInfo = &buffer_descriptor;

		buffer->Update(uniform_data.data, uniform_data.size, buffer_descriptor.offset);
		buffer_count++;
	}

	// Process bindless textures
	{
		auto& texture_queue = texture_to_update[render_context->GetCurrentFrameIndex()];

		u64 texture_count{ 0ull };
		while (!texture_queue.empty() && texture_count < k_max_texture_update_per_frame)
		{
			auto texture_id = texture_queue.front();
			auto& texture = textures[texture_id];

			auto& start_element_index = texture_index_remap[texture_id];
			bool is_storage_image = utl::cast<bool>(texture->GetFlags() & ETextureCreateFlagBits::eStorage);

			// TODO TEXTURE
			auto& mip_level = texture->GetImageView();
			auto& image_descriptor = image_descriptors[texture_count];
			image_descriptor.imageView = Handle(mip_level);
			image_descriptor.sampler = Handle(*(texture->GetSamplerID() != ~0u ? sampler_cache[texture->GetSamplerID()] : default_sampler));
			image_descriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (texture->GetFlags() & ETextureCreateFlagBits::eSamplable)
			{
				auto& write_descriptor_set = write_descriptor_sets.emplace_back();
				write_descriptor_set.dstArrayElement = start_element_index;
				write_descriptor_set.descriptorType = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_set.dstSet = dest_set.GetHandle();
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
	auto format = lookup_vk_format(texture_ci.format);

	auto texture_builder = TextureBuilder();

	texture_builder.SetFormat(format);
	texture_builder.SetFlags(texture_ci.flags);
	texture_builder.SetExtent({ texture_ci.size.width, texture_ci.size.height, texture_ci.size.depth });
	texture_builder.SetSizingPolicy(texture_ci.size.policy);
	texture_builder.SetScaleFactor(texture_ci.size.scale);
	texture_builder.SetMipCount(texture_ci.mipLevels);
	texture_builder.SetLayerCount(texture_ci.arrayLayers);
	texture_builder.SetDebugName(name);

	auto sampler_builder = SamplerBuilder();
	sampler_builder.SetAddressMode(vk::SamplerAddressMode::eRepeat);
	sampler_builder.SetEnableAnisotropy(true);
	sampler_builder.SetCompareEnable(is_depth_format(format));
	sampler_builder.SetMinFilter(vk::Filter::eNearest);
	sampler_builder.SetMagFilter(vk::Filter::eNearest);
	sampler_builder.SetMinLod(0.f);
	sampler_builder.SetMaxLod(1000.f);
	sampler_builder.SetDebugName(name);

	if (texture_ci.flags & ETextureCreateFlagBits::eRenderTarget)
	{
		sampler_builder.SetAddressMode(vk::SamplerAddressMode::eClampToEdge);
		sampler_builder.SetEnableAnisotropy(false);
	}

	auto sampler_hash = sampler_builder.Hash();
	if (auto found = sampler_cache.find(sampler_hash); found == sampler_cache.end())
	{
		sampler_cache[sampler_hash] = sampler_builder.BuildUnique(*render_device);
	}

	texture_builder.SetSampler(sampler_hash);

	// Create texture handle
	return FAPIHandle(render_resources->add<Texture>(name.c_str(), texture_builder.BuildUnique(*render_device)));
}

FAPIHandle VkRenderBackend::CreateTexture(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info)
{
	auto texture_id = CreateTexture(name, texture_ci);
	auto& texture = render_resources->get<Texture>(texture_id.Get());

	upload_context->UpdateTexture(texture.get(), upload_info);
	
	return texture_id;
}

FAPIHandle VkRenderBackend::GetTextureId(const std::string& name)
{
	return FAPIHandle(render_resources->getId<Texture>(name.c_str()));
}

FTextureView VkRenderBackend::GetTextureView(FAPIHandle texture_id)
{
	auto& texture = render_resources->get<Texture>(texture_id.Get());

	const auto& texture_extent = texture->GetImage().GetExtent();

	FTextureView tex_view{};
	tex_view.width = texture_extent.width;
	tex_view.height = texture_extent.height;
	tex_view.depth = texture_extent.depth;
	tex_view.arrayLayers = texture->GetImage().GetArrayLayerCount();
	tex_view.mipLevels = texture->GetImage().GetMipLevelCount();

	return tex_view;
}

void VkRenderBackend::DestroyTexture(FAPIHandle resource_id)
{
	resource_destroy_queue.push(FAPIObjectDestroyInfo{ resource_id, EResourceFlagBits::eTexture });
}

void VkRenderBackend::ReadPixel(FAPIHandle resource_id, u32 x, u32 y, void* data)
{
	static constexpr u64 pixel_size{ 4ull };

	auto& texture = render_resources->get<Texture>(resource_id.Get());

	// Create temporary command buffer
	auto& command_buffer = command_pool->RequestCommandBuffer(vk::CommandBufferLevel::ePrimary);
	command_buffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	auto builder = BufferBuilder(pixel_size, 1ull, 1ull);
	builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
	builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
	builder.SetSharingMode(vk::SharingMode::eConcurrent);

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

	// TODO TEXTURE/COMMAND BUFFER
	Handle(command_buffer).copyImageToBuffer(Handle(texture->GetImage()), vk::ImageLayout::eTransferSrcOptimal, Handle(*staging_buffer), 1, &region);

	render_device->SubmitCommandBuffer(Handle(command_buffer));
	// TODO: DESTROY COMMAND BUFFER AND DELETE IT

	// Copy data to buffer
	void* mapped_buffer_memory = staging_buffer->Map();
	util::Copy(data, mapped_buffer_memory, pixel_size);
	staging_buffer->Unmap();
}


FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci, const std::vector<FShaderCreationBlockInfo>& shader_stages)
{
	const char* shaderEntryPoint{ "main" };

	auto shader_builder = ShaderBuilder();
	shader_builder.SetBindPoint(vk::PipelineBindPoint::eGraphics);

	// TODO: Add alpha blend support?
	shader_builder.graphics_pipeline_builder.SetDebugName(name);
	shader_builder.graphics_pipeline_builder.SetPrimitiveTopology(get_primitive_topology(shader_ci.primitiveTopology));
	shader_builder.graphics_pipeline_builder.SetCullMode(get_cull_mode_flags(shader_ci.cullMode));
	shader_builder.graphics_pipeline_builder.SetFrontFace(get_front_face(shader_ci.frontFace));
	shader_builder.graphics_pipeline_builder.SetPolygonMode(vk::PolygonMode::eFill);
	shader_builder.graphics_pipeline_builder.SetDepthCompareOp(get_compare_op(shader_ci.depthCompareOp));
	shader_builder.graphics_pipeline_builder.SetSampleCount(vk::SampleCountFlagBits::e1);
	shader_builder.graphics_pipeline_builder.SetPatchControlPointsCount(0);
	shader_builder.graphics_pipeline_builder.SetLineWidth(1.f);

	shader_builder.graphics_pipeline_builder.SetDepthClampEnable(true);

	shader_builder.graphics_pipeline_builder.SetColorBlendingLogicOpEnable(false);
	shader_builder.graphics_pipeline_builder.SetColorBlendingLogicOp(vk::LogicOp::eCopy);

	// Get color attachment formats
	auto& textures = render_resources->data<Texture>();
	for (auto& attachment : shader_ci.colorAttachments)
	{
		vk::Format rt_format{ vk::Format::eUndefined };
		if (attachment.backbuffer)
		{
			auto& current_frame = render_context->GetCurrentFrame();
			rt_format = current_frame.GetBackbuffer().GetImage().GetFormat();
		}
		else
			rt_format = textures[attachment.source.Get()]->GetImage().GetFormat();

		shader_builder.graphics_pipeline_builder.AddColorAttachmentFormat(rt_format);

		// Add blending state
		vk::PipelineColorBlendAttachmentState state{};
		state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		state.blendEnable = shader_ci.alphaBlending; // TODO: add color blending support

		state.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		state.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		state.colorBlendOp = vk::BlendOp::eAdd;
		state.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;//vk::BlendFactor::eOne;
		state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		state.alphaBlendOp = vk::BlendOp::eAdd;
		
		shader_builder.graphics_pipeline_builder.AddColorBlendingAttachment(state);
		shader_builder.graphics_pipeline_builder.SetDepthClampEnable(false);
	}

	shader_builder.graphics_pipeline_builder.AddDynamicState(vk::DynamicState::eViewport);
	shader_builder.graphics_pipeline_builder.AddDynamicState(vk::DynamicState::eScissor);
	shader_builder.graphics_pipeline_builder.AddDynamicState(vk::DynamicState::ePrimitiveTopology);

	shader_builder.graphics_pipeline_builder.AddViewport({});
	shader_builder.graphics_pipeline_builder.AddScissorRect({});

	shader_builder.graphics_pipeline_builder.EnableDynamicRendering(true);

	// Get depth format
	if (shader_ci.depthAttachment.source)
	{
		auto& texture = textures[shader_ci.depthAttachment.source.Get()];
		shader_builder.graphics_pipeline_builder.SetDepthAttachmentFormat(texture->GetImage().GetFormat());
		shader_builder.graphics_pipeline_builder.SetDepthTestEnable(true);
		shader_builder.graphics_pipeline_builder.SetDepthWriteEnable(true);
	}

	// Get stencil format
	if (shader_ci.stencilAttachment.source)
	{
		auto& texture = textures[shader_ci.stencilAttachment.source.Get()];
		shader_builder.graphics_pipeline_builder.SetStencilAttachmentFormat(texture->GetImage().GetFormat());
	}

	for (auto& vertex_binding : shader_ci.vertexDeclaration.bindingDescriptions)
	{
		vk::VertexInputBindingDescription description{};
		description.binding = vertex_binding.binding;
		description.stride = vertex_binding.stride;
		description.inputRate = vk::VertexInputRate::eVertex;
		shader_builder.graphics_pipeline_builder.AddVertexInputBindingDescription(description);
	}

	for (auto& vertex_binding : shader_ci.vertexDeclaration.attributeDescriptions)
	{
		vk::VertexInputAttributeDescription description{};
		description.location = vertex_binding.location;
		description.binding = vertex_binding.binding;
		description.format = lookup_vk_format(vertex_binding.format);
		description.offset = vertex_binding.offset;
		shader_builder.graphics_pipeline_builder.AddVertexAttributeDescription(description);
	}

	// Create shader modules and collect reflection data
	u32 shader_index{ 0u };
	for (const auto& [stage, code] : shader_stages)
	{
		auto shader_stage = get_shader_stage(stage);
		shader_builder.AddShaderModule(shader_stage, code);
	}

	shader_builder.AddDefaultLayout(Handle(render_context->GetGlobalDescriptorSetLayout()));

	auto shader = shader_builder.BuildUnique(*render_device);
	auto shader_unique_name = create_object_unique_name("shader:{:x}:{}", shader->GetPipeline().GetHandle(), vk::to_string(shader->GetBindPoint()));
	return FAPIHandle(render_resources->add<Shader>(shader_unique_name.c_str(), std::move(shader)));
}

FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FComputeShaderCreateInfo& shader_ci, const FShaderCreationBlockInfo& shader_stage)
{
	auto shader_builder = ShaderBuilder();
	shader_builder.SetBindPoint(vk::PipelineBindPoint::eCompute);

	shader_builder.AddShaderModule(vk::ShaderStageFlagBits::eCompute, shader_stage.code);

	shader_builder.AddDefaultLayout(Handle(render_context->GetGlobalDescriptorSetLayout()));

	auto shader = shader_builder.BuildUnique(*render_device);

	auto shader_unique_name = create_object_unique_name("shader:{:x}:{}", shader->GetPipeline().GetHandle(), vk::to_string(shader->GetBindPoint()));
	return FAPIHandle(render_resources->add<Shader>(shader_unique_name.c_str(), std::move(shader)));
}

FAPIHandle VkRenderBackend::CreateShader(const std::string& name, const FReyTracingShaderCreateInfo& shader_ci)
{
	return FAPIHandle{};
}

FShaderReflectionData VkRenderBackend::GetShaderReflection(FAPIHandle shader_view)
{
	auto& shader = render_resources->get<Shader>(shader_view.Get());
	return shader->GetReflectionData();
}

void VkRenderBackend::UpdateUniform(FAPIHandle shader_id, uint64_t reflection_name, std::span<const char> data)
{
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	shader->UpdateUniform(*render_context, reflection_name, data);
}

void VkRenderBackend::BindTexture(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle texture_id, uint32_t mip_level)
{
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	auto& texture = render_resources->get<Texture>(texture_id.Get());

	auto& sampler = texture->GetSamplerID() != ~0u ? sampler_cache[texture->GetSamplerID()] : default_sampler;
	shader->UpdateTextureBinding(reflection_name, *texture, *sampler, mip_level);
}

void VkRenderBackend::BindBuffer(FAPIHandle shader_id, uint64_t reflection_name, FAPIHandle buffer_id)
{
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());

	shader->UpdateBufferBinding(reflection_name, *buffer);
}

void VkRenderBackend::FlushConstantRanges(FAPIHandle shader_id, const utl::hash_map<u64, FUniformDataView>& constants, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = render_context->GetCurrentCommandBuffer();
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	shader->FlushConstantRanges(command_buffer, constants);
}

void VkRenderBackend::BindShader(FAPIHandle shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = render_context->GetCurrentCommandBuffer();

	// TODO: make possible to select what pipeline i have to bind now
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	shader->Bind(command_buffer);
}

void VkRenderBackend::ResetShaderResources(FAPIHandle shader_id)
{
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	shader->ResetDescriptors(*render_context);
}

void VkRenderBackend::BindShaderResources(FAPIHandle shader_id, EQueueTypeFlagBits queue_type)
{
	auto& command_buffer = render_context->GetCurrentCommandBuffer();
	auto& shader = render_resources->get<Shader>(shader_id.Get());
	shader->BindDescriptors(command_buffer);
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

void VkRenderBackend::BeginUpdateBuffer(BufferUpdateInfo& update_info)
{
	auto& buffer = render_resources->get<Buffer>(update_info.buffer_id.Get());

	VkBufferUpdateInfo vk_update_info{};
	vk_update_info.dst_buffer = buffer.get();
	vk_update_info.update_size = update_info.update_size;
	
	upload_context->BeginBufferUpdate(vk_update_info);

	update_info.data = vk_update_info.map_info.data;
	update_info.mapped_size = vk_update_info.map_info.avaliable_size;
	update_info.buffer_index = vk_update_info.map_info.buffer_index;
}

void VkRenderBackend::EndUpdateBuffer(BufferUpdateInfo& update_info)
{
	auto& buffer = render_resources->get<Buffer>(update_info.buffer_id.Get());

	VkBufferUpdateInfo vk_update_info{};
	vk_update_info.dst_buffer = buffer.get();
	vk_update_info.update_size = update_info.update_size;
	vk_update_info.map_info.data = update_info.data;
	vk_update_info.map_info.avaliable_size = update_info.mapped_size;
	vk_update_info.map_info.buffer_index = update_info.buffer_index;

	vk_update_info.copy_regions.resize(update_info.regions.size());
	for (int32_t i = 0; i < update_info.regions.size(); ++i)
	{
		auto& region_info = update_info.regions[i];
		auto& copy_region = vk_update_info.copy_regions[i];
		copy_region.srcOffset = region_info.srcOffset;
		copy_region.dstOffset = region_info.dstOffset;
		copy_region.size = region_info.size;
	}

	upload_context->EndBufferUpdate(vk_update_info);

	update_info.data = vk_update_info.map_info.data;
	update_info.mapped_size = vk_update_info.map_info.avaliable_size;
	update_info.buffer_index = vk_update_info.map_info.buffer_index;
}

FBufferView VkRenderBackend::GetBufferView(FAPIHandle buffer_id)
{
	auto& buffer = render_resources->get<Buffer>(buffer_id.Get());
	return { buffer->GetSize(), buffer->GetSize() / buffer->GetSuballocationSize(), buffer->GetDeviceAddress()};
}

void VkRenderBackend::BindVertexBuffers(const FVertexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	utl::vector<vk::Buffer> buffers{};
	for (const auto& buffer_id : binding_info.buffers)
		buffers.emplace_back(Handle(*render_resources->get<Buffer>(buffer_id.Get())));

	render_context->GetCurrentCommandBuffer().BindVertexBuffers(binding_info.firstBinding, buffers, binding_info.offsets);
}

void VkRenderBackend::BindIndexBuffer(const FIndexBufferBindingInfo& binding_info, EQueueTypeFlagBits queue_type)
{
	vk::IndexType index_type{ vk::IndexType::eNoneKHR };
	switch (binding_info.indexSize)
	{
	case 4: index_type = vk::IndexType::eUint32; break;
	case 2: index_type = vk::IndexType::eUint16; break;
	case 1: index_type = vk::IndexType::eUint8EXT; break;
	}

	auto& buffer = render_resources->get<Buffer>(binding_info.bufferId.Get());
	Handle(render_context->GetCurrentCommandBuffer()).bindIndexBuffer(Handle(*buffer), binding_info.offset, index_type);
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

void VkRenderBackend::ProcessResourceBarriers(EQueueTypeFlagBits queue, const utl::vector<FResourceBarrierInfo>& barriers)
{
	auto& textures = render_resources->data<Texture>();
	auto& buffers = render_resources->data<Buffer>();

	FVkTranslationInfo translation_info{};

	for (const auto& resource_barrier : barriers)
	{
		ERenderResourceAccessFlagBits src_resource_access{};
		ERenderResourceAccessFlagBits dst_resource_access{};
		unpackDependencyBarrier(resource_barrier.dependencyType, src_resource_access, dst_resource_access);

		if (resource_barrier.barrierType == ERenderResourceBarrierType::eMemory)
		{
			auto& memory_barrier = translation_info.memoryTranslationInfos[translation_info.memory_tanslation_count++];

			if (resource_barrier.resource.type & ERenderResourceTypeFlagBits::eAllImage)
			{
				auto& texture = textures[resource_barrier.resource.handle.Get()];
				texture->MakeImageLayoutTranslation(memory_barrier, resource_barrier);
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
				Texture* texture{ nullptr };
				if (resource_barrier.resource.type & ERenderResourceTypeFlagBits::eBackbuffer)
				{
					auto& current_frame = render_context->GetCurrentFrame();
					texture = &current_frame.GetBackbuffer();
				}
				else
					texture = textures[resource_barrier.resource.handle.Get()].get();

				auto& texture_barrier = translation_info.imageTranslationInfos[translation_info.image_tanslation_count++];
				texture->MakeImageLayoutTranslation(texture_barrier, resource_barrier);
			}
			// For buffers
			else
			{
				auto& buffer = buffers[resource_barrier.resource.handle.Get()];
				auto& buffer_translation_info = translation_info.bufferTranslationInfos[translation_info.buffer_tanslation_count++];
				buffer->MakeTranslation(buffer_translation_info, resource_barrier);
			}
		}
	}

	ProcessMemoryBarriers(render_context->GetCurrentCommandBuffer(), translation_info);
}

void VkRenderBackend::BeginTimestampZone(EQueueTypeFlagBits queue)
{
	//assert(zone_entries[queue] == 0 && "Previous zone was not ended.");
	render_context->GetCurrentCommandBuffer().WriteTimestampQuery(vk::PipelineStageFlagBits::eTopOfPipe, timestamp_query);
}

void VkRenderBackend::EndTimestampZone(EQueueTypeFlagBits queue)
{
	render_context->GetCurrentCommandBuffer().WriteTimestampQuery(vk::PipelineStageFlagBits::eBottomOfPipe, timestamp_query);
}

void VkRenderBackend::BeginRendering(const FBeginRendering2Info& rendering_info)
{
	auto& textures = render_resources->data<Texture>();

	utl::hash_map<u64, vk::ClearValue> clear_values{};
	for (const auto& [handle, clear_value] : rendering_info.clearValues)
	{
		auto& new_value = clear_values[handle.Get()];

		if (handle)
		{
			auto& texture = textures[handle.Get()];

			auto aspect = texture->GetImageView().GetSubresourceRange().aspectMask;
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

			Texture* texture{ nullptr };
			if (color_attachment_info.backbuffer)
			{
				auto& current_frame = render_context->GetCurrentFrame();
				texture = &current_frame.GetBackbuffer();
			}
			else
				texture = textures[color_attachment_info.handle.Get()].get();

			auto& target_subresource = texture->GetImageView(color_attachment_info.mipLevel);
			attachment_info.imageView = Handle(target_subresource);
			attachment_info.imageLayout = target_subresource.GetImageLayout();

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
		auto& target_subresource = depth_target->GetImageView(rendering_info.depthAttachment->mipLevel);

		depth_attachment.imageView = Handle(target_subresource);
		depth_attachment.imageLayout = target_subresource.GetImageLayout();
		depth_attachment.resolveMode = vk::ResolveModeFlagBits::eNone;
		depth_attachment.loadOp = get_attachment_load_op(rendering_info.depthAttachment->loadOp);
		depth_attachment.storeOp = get_attachment_store_op(rendering_info.depthAttachment->storeOp);
		depth_attachment.clearValue = clear_values[rendering_info.depthAttachment->handle.Get()];
	}

	vk::RenderingAttachmentInfoKHR stencil_attachment{};
	if (rendering_info.stencilAttachment && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle && rendering_info.stencilAttachment->handle)
	{
		auto& stencil_target = textures[rendering_info.stencilAttachment->handle.Get()];
		auto& target_subresource = stencil_target->GetImageView(rendering_info.stencilAttachment->mipLevel);

		stencil_attachment.imageView = Handle(target_subresource);
		stencil_attachment.imageLayout = target_subresource.GetImageLayout();
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
		if (!is_depth_only_format(depth_target->GetImage().GetFormat()) && rendering_info.stencilAttachment->handle == rendering_info.depthAttachment->handle)
			begin_rendering_info.pStencilAttachment = &depth_attachment;
	}

	if (rendering_info.stencilAttachment->handle && rendering_info.stencilAttachment->handle != rendering_info.depthAttachment->handle)
		begin_rendering_info.pStencilAttachment = &stencil_attachment;

	render_context->GetCurrentCommandBuffer().BeginDebugLabel(rendering_info.name, 0x5555ffff);

	Handle(render_context->GetCurrentCommandBuffer()).beginRenderingKHR(begin_rendering_info);
}

void VkRenderBackend::EndRendering(EQueueTypeFlagBits queue)
{
	Handle(render_context->GetCurrentCommandBuffer()).endRenderingKHR();
	render_context->GetCurrentCommandBuffer().EndDebugLabel();
}

void VkRenderBackend::BeginCompute(const FBeginComputeInfo& compute_info)
{
	render_context->GetCurrentCommandBuffer().BeginDebugLabel(compute_info.name, 0x55ff55ff);
}

void VkRenderBackend::EndCompute(EQueueTypeFlagBits queue)
{
	render_context->GetCurrentCommandBuffer().EndDebugLabel();
}

void VkRenderBackend::ProcessMemoryBarriers(CommandBuffer& command_buffer, const FVkTranslationInfo& translation_info)
{
	vk::DependencyInfoKHR dependency_info{};
	dependency_info.imageMemoryBarrierCount = utl::cast<u32>(translation_info.image_tanslation_count);
	dependency_info.pImageMemoryBarriers = translation_info.imageTranslationInfos.data();
	dependency_info.bufferMemoryBarrierCount = utl::cast<u32>(translation_info.buffer_tanslation_count);
	dependency_info.pBufferMemoryBarriers = translation_info.bufferTranslationInfos.data();
	dependency_info.memoryBarrierCount = utl::cast<u32>(translation_info.memory_tanslation_count);
	dependency_info.pMemoryBarriers = translation_info.memoryTranslationInfos.data();
	dependency_info.dependencyFlags = vk::DependencyFlagBits::eByRegion;

	command_buffer.PipelineBarrier(dependency_info);
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

	Handle(render_context->GetCurrentCommandBuffer()).setViewport(0, 1, &viewport);
}

void VkRenderBackend::SetScissorRect(i32 offset_x, i32 offset_y, u32 width, u32 height)
{
	vk::Rect2D scissor_rect{};
	scissor_rect.offset.x = offset_x;
	scissor_rect.offset.y = offset_y;
	scissor_rect.extent.width = width;
	scissor_rect.extent.height = height;

	Handle(render_context->GetCurrentCommandBuffer()).setScissor(0, 1, &scissor_rect);
}

void VkRenderBackend::SetPrimitiveTopology(EVertexPrimitiveTopology primitivd_topology)
{
	Handle(render_context->GetCurrentCommandBuffer()).setPrimitiveTopology(get_primitive_topology(primitivd_topology));
}

// TODO: add indirect Draw support
void VkRenderBackend::Draw(EQueueTypeFlagBits queue_type, const FGeometryDrawInfo& draw_info)
{
	if (draw_info.lastIndex == 0ull && draw_info.lastVertex == 0ull)
	{
		auto& range = draw_info.ranges.front();
		Handle(render_context->GetCurrentCommandBuffer()).draw(range.vertexCount, 1, range.beginVertex, 0);
	}
	else if (draw_info.lastIndex == 0ull)
	{
		for (auto& range : draw_info.ranges)
		{
			auto vertex_count = range.vertexCount == 0ull ? draw_info.lastIndex : range.vertexCount;
			Handle(render_context->GetCurrentCommandBuffer()).draw(vertex_count, 1, range.beginVertex, 0);
		}
	}
	else
	{
		for (auto& range : draw_info.ranges)
		{
			auto index_count = range.indexCount == 0ull ? draw_info.lastIndex : range.indexCount;
			Handle(render_context->GetCurrentCommandBuffer()).drawIndexed(index_count, 1, range.beginIndex, range.beginVertex, 0);
		}
	}
}

void VkRenderBackend::Draw(EQueueTypeFlagBits queue_type, const FIndirectGeometryDrawInfo& draw_info)
{
	auto& indirect_command_buffer = render_resources->get<Buffer>(draw_info.bufferId.Get());
	auto& indirect_count_buffer = render_resources->get<Buffer>(draw_info.countBufferId.Get());
	Handle(render_context->GetCurrentCommandBuffer()).drawIndexedIndirectCount(Handle(*indirect_command_buffer), draw_info.offset, Handle(*indirect_count_buffer), draw_info.countOffset, draw_info.maxDrawCount, draw_info.stride);
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
		auto& shader = render_resources->get<Shader>(dispatch_info.shaderHandle.Get());
		shader->DivideByLocalSizes(command, subgroup_index);
	}

	Handle(render_context->GetCurrentCommandBuffer()).dispatch(command.x, command.y, command.z);
}

void VkRenderBackend::Dispatch(EQueueTypeFlagBits queue_type, const FDispatchIndirectInfo& dispatch_info)
{
	auto& argument_buffer = render_resources->get<Buffer>(dispatch_info.argumentBuffer.Get());

	if (dispatch_info.countBuffer)
	{
		auto& argument_buffer = render_resources->get<Buffer>(dispatch_info.argumentBuffer.Get());
		// Dispatch indirect count is not supported
	}
	else
		Handle(render_context->GetCurrentCommandBuffer()).dispatchIndirect(Handle(*argument_buffer), dispatch_info.argumentOffset);
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
	return FAPIHandle(render_resources->getId<Texture>(name));
}

FAPIHandle VkRenderBackend::GetShaderResource(const char* name)
{
	return FAPIHandle(render_resources->getId<Shader>(name));
}

FPoint2D<u32> VkRenderBackend::GetRenderResolution() const
{
	auto& extent = render_context->GetSwapchain().GetExtent();
	FPoint2D<u32> resolution{};
	resolution.x = extent.width;
	resolution.y = extent.height;

	return resolution;
}

FRect2D<f32> VkRenderBackend::GetRenderRectFromRT(FAPIHandle texture_id)
{
	auto& texture = render_resources->get<Texture>(texture_id.Get());

	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(texture->GetImage().GetExtent().width);
	rect.size.y = utl::cast<f32>(texture->GetImage().GetExtent().height);

	return rect;
}

FRect2D<f32> VkRenderBackend::GetScreenRenderRect()
{
	auto& extent = render_context->GetSwapchain().GetExtent();

	FRect2D<f32> rect{};
	rect.size.x = utl::cast<f32>(extent.width);
	rect.size.y = utl::cast<f32>(extent.height);
	return rect;
}

// TODO: update only targets, that have same size with swapchain
void VkRenderBackend::UpdateSurface()
{
	auto& surface_extent = render_context->GetSwapchain().GetExtent();
	auto& textures = render_resources->data<Texture>();

	// Updating render targets
	for (u64 texture_id = 0ull; texture_id < textures.size(); ++texture_id)
	{
		auto& texture = textures[texture_id];

		if (!(texture->GetFlags() & ETextureCreateFlagBits::eRenderTarget) || (texture->GetSizingPolicy() == ERenderTargetSizePolicy::eFixed))
			continue;

		vk::Extent3D new_extent{ surface_extent, 1u };

		// TODO: here i should set rendering resolution, not swapchain image size
		if (texture->GetSizingPolicy() == ERenderTargetSizePolicy::eScalable)
		{
			new_extent.width *= texture->GetScaleFactor();
			new_extent.height *= texture->GetScaleFactor();
		}

		texture = std::make_unique<Texture>(*texture, new_extent);
	}
}

const std::unique_ptr<RenderDevice>& VkRenderBackend::getDevice() const
{
	return render_device;
}

void VkRenderBackend::BeginFrame()
{
	u32 image_index{ 0u };
	// TODO: sync queues here

	bool surface_updated{ false };
	auto& command_buffer = render_context->Begin(surface_updated);
	if (surface_updated)
	{
		UpdateSurface();
	}

	// TODO: here i should run only main command buffer. But for now i getting the last one
	command_buffer.ResetTimestampQuery(timestamp_query);

	// Update bindless textures
	auto& textures = render_resources->data<Texture>();

	if (texture_index_remap.empty())
	{
		// Resizing remap vector
		texture_index_remap.resize(textures.size());

		u64 image_offset{ 0ull };
		for (u64 texture_index = 0ull; texture_index < textures.size(); ++texture_index)
		{
			auto& texture = textures[texture_index];
			if (texture->GetFlags() & ETextureCreateFlagBits::eRenderTarget)
				continue;

			texture_index_remap[texture_index] = image_offset++;

			for (u32 frame_index = 0u; frame_index < render_context->GetSwapchain().GetImageCount(); ++frame_index)
				texture_to_update[frame_index].push(texture_index);
		}
	}
}

void VkRenderBackend::EndFrame()
{
	// TODO: here i should run only main command buffer. But for now i getting the last one
	//render_device->WriteTimestampQuery(command_buffer, vk::PipelineStageFlagBits::eBottomOfPipe, timestamp_query);

	vk::SemaphoreSubmitInfo upload_wait_semaphore{};
	//auto upload_avaliable = render_context->GetUploadAvaliableSemaphore();
	//upload_wait_semaphore = upload_context->Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
	upload_wait_semaphore = upload_context->Submit(nullptr, 0);

	render_context->EndFrame(upload_wait_semaphore);

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
	
	// Try to free resources
	if (utilization_counter % render_context->GetSwapchain().GetImageCount() == 0)
		ProcessResourceUtilization();

	// Frame finished. Textures in now again in initial layout
	// TODO: i don't need to set layout back to undefined for one time targets or for periodical targets
	auto& textures = render_resources->data<Texture>();
	for (auto& texture : textures)
	{
		if ((texture->GetFlags() & ETextureCreateFlagBits::eRenderTarget) && !(texture->GetFlags() & ETextureCreateFlagBits::eStorage))
		{
			// Reset layout for all mip level
			for(auto& mip : texture->GetImageViews())
				mip->SetImageLayout(vk::ImageLayout::eUndefined);

			// Also reset view that stores all other levels
			texture->GetImageView().SetImageLayout(vk::ImageLayout::eUndefined);
		}
	}

	utilization_counter++;
}

bool VkRenderBackend::IsSwapChainFinished()
{
	return (utilization_counter % render_context->GetSwapchain().GetImageCount() == 0) && utilization_counter != 0ull;
}

void VkRenderBackend::BeginDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	render_context->GetCurrentCommandBuffer().BeginDebugLabel(label, color);
}

void VkRenderBackend::InsertDebugLabel(const std::string& label, EQueueTypeFlagBits queue_type, u32 color)
{
	render_context->GetCurrentCommandBuffer().InsertDebugLabel(label, color);
}

void VkRenderBackend::EndDebugLabel(EQueueTypeFlagBits queue_type)
{
	render_context->GetCurrentCommandBuffer().EndDebugLabel();
}

void VkRenderBackend::ProcessResourceUtilization()
{
	while (!resource_destroy_queue.empty())
	{
		auto destroy_resource = resource_destroy_queue.front();

		if (destroy_resource.mask & EResourceFlagBits::eTexture)
		{
			auto& texture = render_resources->get<Texture>(destroy_resource.id.Get());
			texture.reset();
			render_resources->remove<Texture>(destroy_resource.id.Get());
		}
		else if (destroy_resource.mask & EResourceFlagBits::eShader)
		{
			auto& shader = render_resources->get<Shader>(destroy_resource.id.Get());
			shader.reset();
			render_resources->remove<Shader>(destroy_resource.id.Get());
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

void VkRenderBackend::ProcessImageLayoutTranslation(CommandBuffer& command_buffer, const vk::ImageMemoryBarrier2KHR& image_translation_info)
{
	FVkTranslationInfo translation_info{};
	translation_info.imageTranslationInfos[translation_info.image_tanslation_count++] = image_translation_info;
	ProcessMemoryBarriers(command_buffer, translation_info);
}

void VkRenderBackend::TextureLayoutTranslation(CommandBuffer& command_buffer, Texture& texture, const FVkTextureTranslationInfo& translation_info)
{
	auto image_layout_translation = texture.GetTranslationInfo(translation_info);
	ProcessImageLayoutTranslation(command_buffer, image_layout_translation);
}
#include <engine/render/render_api/vulkan/UploadContext.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/CommandPool.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/Buffer.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Texture.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			UploadContext::UploadContext(RenderDevice& render_device) :
				render_device{ render_device },
				queue{ render_device.GetLogicalDevice().GetQueueByFlags(vk::QueueFlagBits::eGraphics, 1u) }
			{
				auto command_pool_builder = CommandPoolBuilder();
				command_pool_builder.SetDebugName("Upload");
				command_pool_builder.SetResetMode(CommandBuffer::ResetMode::ResetIndividually);
				command_pool_builder.SetQueueFamily(queue.GetFamilyIndex());
				command_pool = command_pool_builder.BuildUnique(render_device);

				vk::SemaphoreCreateInfo semaphore_ci{};
				signal_semaphore = render_device.CreateSemaphore(semaphore_ci);
			}

			UploadContext::~UploadContext()
			{

			}

			void UploadContext::UpdateTexture(Texture* texture_handle, const FTextureUploadInfo& upload_info)
			{
				BeginCommands();

				auto& target_subresource = texture_handle->GetImageView();

				vk::DependencyInfoKHR dependency_info{};

				FVkTextureTranslationInfo texture_translation_info{};
				texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
				texture_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;
				target_subresource.SetImageLayout(vk::ImageLayout::eTransferDstOptimal);

				auto image_translation = texture_handle->GetTranslationInfo(texture_translation_info);

				dependency_info.imageMemoryBarrierCount = 1u;
				dependency_info.pImageMemoryBarriers = &image_translation;

				active_command_buffer->PipelineBarrier(dependency_info);


				auto builder = BufferBuilder(upload_info.upload_data.size(), 1ull, 1ull);
				builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
				builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
				builder.SetSharingMode(vk::SharingMode::eConcurrent);

				auto& staging_buffer = upload_buffers.emplace_back(builder.BuildUnique(render_device));

				// Copy data to buffer
				staging_buffer->Update(upload_info.upload_data);

				auto aspect_mask = texture_handle->GetImageView().GetSubresourceRange().aspectMask;
				auto& image = texture_handle->GetImage();
				auto extent = image.GetExtent();

				utl::vector<vk::BufferImageCopy2KHR> copy_regions{};
				if (!upload_info.mip_layer_offsets.empty())
				{
					for (u32 layer = 0u; layer < image.GetArrayLayerCount(); ++layer)
					{
						auto& layer_offsets = upload_info.mip_layer_offsets[layer];
						for (u32 mip = 0u; mip < image.GetMipLevelCount(); ++mip)
						{
							u64 offset = layer_offsets[mip];
							auto& copy_region = copy_regions.emplace_back();
							copy_region.imageSubresource.aspectMask = aspect_mask;
							copy_region.imageSubresource.mipLevel = mip;
							copy_region.imageSubresource.baseArrayLayer = layer;
							copy_region.imageSubresource.layerCount = 1u;
							copy_region.imageExtent.width = extent.width >> mip;
							copy_region.imageExtent.height = extent.height >> mip;
							copy_region.imageExtent.depth = extent.depth;
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
					copy_region.imageSubresource.layerCount = image.GetArrayLayerCount();
					copy_region.imageExtent.width = extent.width;
					copy_region.imageExtent.height = extent.height;
					copy_region.imageExtent.depth = extent.depth;
					copy_region.bufferOffset = 0ull;
				}

				staging_buffer->CopyTo(Handle(*active_command_buffer), Handle(image), target_subresource.GetImageLayout(), copy_regions);

				if (image.GetMipLevelCount() > 1u && upload_info.mip_layer_offsets.empty())
					texture_handle->GenerateMipMaps(Handle(*active_command_buffer));

				texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				target_subresource.SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

				image_translation = texture_handle->GetTranslationInfo(texture_translation_info);

				dependency_info.imageMemoryBarrierCount = 1u;
				dependency_info.pImageMemoryBarriers = &image_translation;

				active_command_buffer->PipelineBarrier(dependency_info);
			}

			void UploadContext::UploadBuffer(Buffer* buffer_handle, const FBufferUploadInfo& upload_info)
			{
				BeginCommands();

				// Create new staging buffer
				auto builder = BufferBuilder(upload_info.upload_size, 1ull, 1ull);
				builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
				builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
				builder.SetSharingMode(vk::SharingMode::eConcurrent);

				builder.SetDebugName(std::format("staging for #{}", upload_info.buffer_id.Get()));

				auto& src_buffer = upload_buffers.emplace_back(builder.BuildUnique(render_device));

				// Copy upload data to buffer
				src_buffer->Update(upload_info.upload_data);

				utl::vector<vk::BufferCopy2KHR> copy_regions(upload_info.regions.size());

				auto* dst_buffer = buffer_handle;

				for (int32_t idx = 0; idx < copy_regions.size(); ++idx)
				{
					auto& region_info = upload_info.regions[idx];

					auto& copy_region = copy_regions[idx];
					copy_region.srcOffset = region_info.srcOffset;
					copy_region.dstOffset = region_info.dstOffset;
					copy_region.size = region_info.size;
				}

				auto& staging_buffer = upload_buffers[upload_info.upload_index];
				src_buffer->CopyTo(Handle(*active_command_buffer), *dst_buffer, copy_regions);
			}

			vk::Semaphore UploadContext::Submit()
			{
				return signal_semaphore;
			}

			void UploadContext::BeginCommands()
			{
				if (!writing_commands)
				{
					auto& command_buffer = command_pool->RequestCommandBuffer(vk::CommandBufferLevel::ePrimary);
					command_buffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
					active_command_buffer = &command_buffer;

					writing_commands = true;
				}
			}
		}
	}
}
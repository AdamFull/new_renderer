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
				upload_frames.emplace_back(std::make_unique<UploadFrame>(render_device, queue));
				//upload_frames.emplace_back(std::make_unique<UploadFrame>(render_device, queue));
			}

			UploadContext::~UploadContext()
			{

			}

			void UploadContext::UpdateTexture(Texture* texture_handle, const FTextureUploadInfo& upload_info)
			{
				BeginFrame();

				auto& upload_frame = GetActiveFrame();
				auto& command_buffer = upload_frame.AcquireOrGetCommandBuffer();

				auto& target_subresource = texture_handle->GetImageView();

				vk::DependencyInfoKHR dependency_info{};

				FVkTextureTranslationInfo texture_translation_info{};
				texture_translation_info.oldLayout = vk::ImageLayout::eUndefined;
				texture_translation_info.newLayout = vk::ImageLayout::eTransferDstOptimal;
				target_subresource.SetImageLayout(vk::ImageLayout::eTransferDstOptimal);

				auto image_translation = texture_handle->GetTranslationInfo(texture_translation_info);

				dependency_info.imageMemoryBarrierCount = 1u;
				dependency_info.pImageMemoryBarriers = &image_translation;

				command_buffer.PipelineBarrier(dependency_info);


				auto builder = BufferBuilder(upload_info.upload_data.size(), 1ull, 1ull);
				builder.SetVmaUsage(vma::MemoryUsage::eCpuOnly);
				builder.SetUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
				builder.SetSharingMode(vk::SharingMode::eConcurrent);

				auto& staging_buffer = upload_frame.RequestStagingBuffer(upload_info.upload_data.size());

				// Copy data to buffer
				staging_buffer.Update(upload_info.upload_data);

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

				staging_buffer.CopyTo(Handle(command_buffer), Handle(image), target_subresource.GetImageLayout(), copy_regions);

				if (image.GetMipLevelCount() > 1u && upload_info.mip_layer_offsets.empty())
					texture_handle->GenerateMipMaps(Handle(command_buffer));

				texture_translation_info.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				texture_translation_info.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				target_subresource.SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

				image_translation = texture_handle->GetTranslationInfo(texture_translation_info);

				dependency_info.imageMemoryBarrierCount = 1u;
				dependency_info.pImageMemoryBarriers = &image_translation;

				command_buffer.PipelineBarrier(dependency_info);
			}

			void UploadContext::BeginBufferUpdate(VkBufferUpdateInfo& update_info)
			{
				BeginFrame();

				Buffer* dst_buffer = update_info.dst_buffer;

				if (dst_buffer->GetUsage() != vma::MemoryUsage::eGpuOnly)
				{
					log_warning("This buffer can be updated from the CPU, no additional memory needs to be allocated.");
					update_info.map_info.data = dst_buffer->Map();
					update_info.map_info.avaliable_size = dst_buffer->GetSize();
				}
				else
				{
					vk::DeviceSize allocation_size = update_info.update_size > 0ull ? update_info.update_size : dst_buffer->GetSize();

					auto& upload_frame = GetActiveFrame();
					auto& src_buffer = upload_frame.RequestStagingBuffer(allocation_size, &update_info.map_info.buffer_index);

					update_info.map_info.data = src_buffer.Map();
					update_info.map_info.avaliable_size = src_buffer.GetSize();
				}
			}

			void UploadContext::EndBufferUpdate(VkBufferUpdateInfo& update_info)
			{
				Buffer* dst_buffer = update_info.dst_buffer;

				if (update_info.map_info.avaliable_size == ~0ull)
				{
					dst_buffer->Unmap();
				}
				else
				{
					auto& upload_frame = GetActiveFrame();
					auto& command_buffer = upload_frame.AcquireOrGetCommandBuffer();
					auto& src_buffer = upload_frame.GetStagingBuffer(update_info.map_info.buffer_index);

					// TODO: Add memory barrier befor copy for dst buffer
					vk::BufferMemoryBarrier2KHR buffer_barrier{};
					buffer_barrier.buffer = Handle(*dst_buffer);
					buffer_barrier.size = dst_buffer->GetSize();
					buffer_barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
					buffer_barrier.dstAccessMask = vk::AccessFlagBits2::eMemoryWrite;

					vk::DependencyInfoKHR dependency_info{};
					dependency_info.pBufferMemoryBarriers = &buffer_barrier;
					dependency_info.bufferMemoryBarrierCount = 1u;

					command_buffer.PipelineBarrier(dependency_info);

					src_buffer.CopyTo(Handle(command_buffer), *dst_buffer, update_info.copy_regions);
				}
			}

			vk::SemaphoreSubmitInfo UploadContext::Submit(vk::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
			{
				if (writing_commands)
				{
					auto& upload_frame = GetActiveFrame();
					auto& command_buffer = upload_frame.AcquireOrGetCommandBuffer();

					command_buffer.EndDebugLabel();
					command_buffer.End();
					writing_commands = false;

					auto signal_semaphore_info = upload_frame.Submit(wait_infos, wait_semaphore_count);

					active_upload_frame = (active_upload_frame + 1u) % (uint32_t)upload_frames.size();

					return signal_semaphore_info;
				}

				return {};
			}

			UploadFrame& UploadContext::GetActiveFrame()
			{
				return *upload_frames[active_upload_frame];
			}

			void UploadContext::BeginFrame()
			{
				if (!writing_commands)
				{
					auto& upload_frame = GetActiveFrame();
					upload_frame.Reset();

					auto& command_buffer = upload_frame.AcquireOrGetCommandBuffer();

					command_buffer.BeginDebugLabel("Upload resources", 0xFFFFFFFF);

					writing_commands = true;
				}
			}
		}
	}
}
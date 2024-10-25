#include <engine/rendering/UploadContext.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include <engine/rendering/native_format/vk_format.h>

#include <engine/filesystem/vfs_helper.h>
#include <util/Logger.h>

#include <gfx/Context.h>

namespace engine
{
	namespace render
	{
		UploadContext::UploadContext(gfx::Context& context, filesystem::IVirtualFileSystemInterface* vfs) :
			context{ context },
			vfs{ vfs }
		{
			queue = context.RequestQueue(gfx::QueueTypeFlag::eGraphics, 1u);

			assert(k_max_upload_frames > upload_frame_count);

			for (int32_t i = 0; i < upload_frame_count; ++i)
			{
				upload_frames[i] = std::make_unique<UploadFrame>(context, *queue);
			}
		}

		gfx::Texture* UploadContext::LoadTexture(const std::string& texture_path)
		{
			BeginFrame();
			using namespace utl;

			auto name = filesystem::fs::get_stem(texture_path);
			auto ext = filesystem::fs::get_ext(texture_path);
			auto ext_hash = utl::murmur_hash(ext);

			std::vector<uint8_t> data{};
			vfs->ReadFile(texture_path, data);

			gfx::TextureCreateInfo create_info{};
			gfx::CopyTextureInfo copy_texture{};

			create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;

			auto& upload_frame = GetActiveFrame();

			switch (ext_hash)
			{
			case ".jpg"_hash32:
			case ".jpeg"_hash32:
			case ".png"_hash32:
			case ".tga"_hash32:
			case ".bmp"_hash32:
			case ".psd"_hash32:
			case ".gif"_hash32:
			case ".hdr"_hash32:
			case ".pic"_hash32:
			case ".pnm"_hash32:
			{
				int32_t width{ 0 }, height{ 0 }, channels{ 0 };
				uint8_t* rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);
				uint64_t data_size = width * height * 4ull;

				create_info.width = width;
				create_info.height = height;
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				
				// TODO: Add mip level generation
				create_info.levels = 1; utl::cast<u32>(std::floor(std::log2((std::max)(width, height)))) + 1u;

				// Create staging buffer for uploading
				auto& src_buffer = upload_frame.RequestStagingBuffer(data_size);

				// Copy image data to staging buffer
				src_buffer.Update(rawdata, data_size);
				stbi_image_free(rawdata);

				copy_texture.src_buffer = &src_buffer;

				auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
				region.size_x = width;
				region.size_y = height;
			} break;
			case ".ktx"_hash32:
			{
				ktxTexture* kTexture{ nullptr };
				auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
				assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
				if (ktxresult != KTX_SUCCESS)
					return nullptr;

				if (kTexture->isCubemap)
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;

				create_info.width = kTexture->baseWidth;
				create_info.height = kTexture->baseHeight;
				create_info.depth = kTexture->baseDepth;

				create_info.levels = kTexture->numLevels;
				create_info.layers = kTexture->numLayers;

				// Get texture format
				auto* ktx1 = (ktxTexture1*)kTexture;

				uint32_t vk_format{};
				vk_format = vkGetFormatFromOpenGLInternalFormat(ktx1->glInternalformat);
				if (vk_format == 0u)
					vk_format = vkGetFormatFromOpenGLFormat(ktx1->glFormat, ktx1->glType);

				create_info.format = gfx::FormatFromVk(vk_format);

				// Create staging buffer for uploading
				auto& src_buffer = upload_frame.RequestStagingBuffer(kTexture->dataSize);
				src_buffer.Update(kTexture->pData, kTexture->dataSize);
				
				// Collect image copy regions
				auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
				for (u32 layer = 0; layer < num_layers; layer++)
				{
					for (u32 level = 0u; level < kTexture->numLevels; level++)
					{
						auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
						region.layer = layer;
						region.level = level;
						region.layer_count = kTexture->numLayers;

						region.size_x = create_info.width >> level;
						region.size_z = create_info.height >> level;

						ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &region.buffer_offset);
					}
				}

				ktxTexture_Destroy(kTexture);
			} break;
			case ".ktx2"_hash32:
			{
				ktxTexture2* kTexture{ nullptr };
				auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);
				assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
				if (ktxresult != KTX_SUCCESS)
					return nullptr;

				if (kTexture->isCubemap)
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;

				create_info.width = kTexture->baseWidth;
				create_info.height = kTexture->baseHeight;
				create_info.depth = kTexture->baseDepth;

				create_info.levels = kTexture->numLevels;
				create_info.layers = kTexture->numLayers;

				if (ktxTexture2_NeedsTranscoding(kTexture))
					ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

				create_info.format = gfx::FormatFromVk(kTexture->vkFormat);

				// Create staging buffer for uploading
				auto& src_buffer = upload_frame.RequestStagingBuffer(kTexture->dataSize);
				src_buffer.Update(kTexture->pData, kTexture->dataSize);

				// Collect image copy regions
				auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
				for (u32 layer = 0; layer < num_layers; layer++)
				{
					for (u32 level = 0u; level < kTexture->numLevels; level++)
					{
						auto& region = copy_texture.copy_regions[copy_texture.copy_region_count++];
						region.layer = layer;
						region.level = level;
						region.layer_count = kTexture->numLayers;

						region.size_x = create_info.width >> level;
						region.size_z = create_info.height >> level;

						ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &region.buffer_offset);
					}
				}

				ktxTexture_Destroy((ktxTexture*)kTexture);
			} break;
			}

			gfx::Texture* texture = context.CreateTexture(create_info);
			copy_texture.dst_texture = texture;

			auto& command_list = upload_frame.AcquireOrGetCommandList();

			gfx::TextureBarrier texture_barrier{};
			texture_barrier.texture = texture;
			texture_barrier.src_access = { {}, gfx::TextureLayout::eUndefined };
			texture_barrier.dst_access = { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eTransfer };

			gfx::ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;

			// Transfer to intermediate layout
			command_list.ResourceBarrier(resource_barrier);

			// Copy texture regions
			command_list.CopyTexture(copy_texture);

			// Transfer to final layout
			texture_barrier.src_access = { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eTransfer };
			texture_barrier.dst_access = { gfx::AccessTypeFlag::eRead, gfx::TextureLayout::eShader };
			command_list.ResourceBarrier(resource_barrier);

			return texture;
		}

		void UploadContext::BeginBufferUpdate(gfx::BufferUpdateInfo& update_info)
		{
			BeginFrame();

			gfx::Buffer* dst_buffer = update_info.dst_buffer;
			
			if (!(dst_buffer->GetUsageFlags() & gfx::BufferUsageFlag::eGpuOnly))
			{
				log_warning("This buffer can be updated from the CPU, no additional memory needs to be allocated.");
				update_info.map_info.data = dst_buffer->Map();
				update_info.map_info.avaliable_size = dst_buffer->GetSize();
			}
			else
			{
				uint32_t allocation_size = update_info.update_size > 0ull ? update_info.update_size : dst_buffer->GetSize();
			
				auto& upload_frame = GetActiveFrame();
				auto& src_buffer = upload_frame.RequestStagingBuffer(allocation_size, &update_info.map_info.buffer_index);
			
				update_info.map_info.data = src_buffer.Map();
				update_info.map_info.avaliable_size = src_buffer.GetSize();
			}
		}

		gfx::Texture* UploadContext::CreateTexture(const gfx::TextureCreateInfo& create_info)
		{
			gfx::Texture* texture = context.CreateTexture(create_info);
			// TODO: add resource registry
			return texture;
		}

		gfx::Sampler* UploadContext::CreateSampler(const gfx::SamplerCreateInfo& create_info)
		{
			gfx::Sampler* sampler = context.CreateSampler(create_info);
			// TODO: add resource registry
			return sampler;
		}

		gfx::Buffer* UploadContext::CreateBuffer(const gfx::BufferCreateInfo& create_info)
		{
			gfx::Buffer* buffer = context.CreateBuffer(create_info);
			// TODO: add resource registry
			return buffer;
		}

		void UploadContext::DestroyResource(gfx::GFXResource* resource)
		{
			context.Destroy(resource);
		}

		void UploadContext::EndBufferUpdate(gfx::BufferUpdateInfo& update_info)
		{
			gfx::Buffer* dst_buffer = update_info.dst_buffer;

			if (update_info.map_info.avaliable_size == ~0ull)
			{
				dst_buffer->Unmap();
			}
			else
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_list = upload_frame.AcquireOrGetCommandList();
				auto& src_buffer = upload_frame.GetStagingBuffer(update_info.map_info.buffer_index);

				// TODO: Add memory barrier befor copy for dst buffer
				gfx::BufferBarrier buffer_barrier{};
				buffer_barrier.buffer = dst_buffer;
				buffer_barrier.src_state.stage = gfx::StageFlag::eAllCommands;
				buffer_barrier.src_state.access =	gfx::AccessFlag::eTransferWrite | gfx::AccessFlag::eShaderWrite | gfx::AccessFlag::eMemoryWrite | gfx::AccessFlag::eShaderStorageWrite;
				buffer_barrier.dst_state.stage = gfx::StageFlag::eTransfer;
				buffer_barrier.dst_state.access = gfx::AccessFlag::eTransferWrite;

				gfx::ResourceBarrierInfo barrier_info{};
				barrier_info.buffer_barriers = &buffer_barrier;
				barrier_info.buffer_barrier_count = 1u;

				command_list.ResourceBarrier(barrier_info);

				gfx::CopyBufferInfo copy_buffer{};
				copy_buffer.src_buffer = &src_buffer;
				copy_buffer.dst_buffer = dst_buffer;

				copy_buffer.regions = update_info.regions;
				copy_buffer.region_count = update_info.region_count;

				command_list.CopyBuffer(copy_buffer);

				update_info.map_info.buffer_index = ~0ull;
			}

			update_info.map_info.data = nullptr;
			update_info.map_info.avaliable_size = 0ull;
		}

		gfx::SemaphoreSubmitInfo UploadContext::Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
		{
			if (writing_commands)
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_list = upload_frame.AcquireOrGetCommandList();

				command_list.EndMarker();
				command_list.End();
				writing_commands = false;

				auto signal_semaphore_info = upload_frame.Submit(wait_infos, wait_semaphore_count);

				active_upload_frame = (active_upload_frame + 1u) % upload_frame_count;

				return signal_semaphore_info;
			}

			return {};
		}

		gfx::Context& UploadContext::GetContext()
		{
			return context;
		}

		gfx::Context const& UploadContext::GetContext() const
		{
			return context;
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

				auto& command_list = upload_frame.AcquireOrGetCommandList();

				command_list.Begin();
				command_list.BeginMarker("Upload resources", 0xFFFFFFFF);

				writing_commands = true;
			}
		}
	}
}
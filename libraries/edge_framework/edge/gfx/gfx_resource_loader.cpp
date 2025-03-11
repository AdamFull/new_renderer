#include "gfx_resource_loader.h"
#include "gfx_context.h"

#include "../util/fs_helper.h"
#include "../util/Logger.h"
#include "../util/Profiler.h"
#include "../util/util.h"

#include "../fs/fs_virtual_filesystem.h"

#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include "MipGeneratorShader.h"
#include "MipGeneratorShaderMip2.h"
#include "MipGeneratorShaderMip3.h"
#include "MipGeneratorShaderMip4.h"

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace edge
{
	namespace gfx
	{
		// Helper class for manage staging memory allocation
		class BlockAllocator
		{
		public:
			struct Block
			{
				IBuffer* buffer;
				uint64_t used;
			};

			BlockAllocator(IContext* gfx_context, uint64_t size) :
				gfx_context{ gfx_context },
				block_size{ size }
			{
				AddBlock(size);
			}

			BlockAllocator(BlockAllocator&& other) noexcept :
				gfx_context{ std::exchange(other.gfx_context, nullptr) },
				block_size{ std::exchange(other.block_size, {}) },
				blocks{ std::exchange(other.blocks, {}) }
			{
			}

			auto operator=(BlockAllocator&& other) noexcept -> BlockAllocator&
			{
				gfx_context = std::exchange(other.gfx_context, nullptr);
				block_size = std::exchange(other.block_size, {});
				blocks = std::exchange(other.blocks, {});

				return* this;
			}

			BlockAllocator(const BlockAllocator&) = delete;
			BlockAllocator& operator=(const BlockAllocator&) = delete;

			~BlockAllocator()
			{
				for (auto& block : blocks)
				{
					gfx_context->Destroy(block.buffer);
				}
			}

			auto Allocate(uint64_t requested_size) -> std::tuple<IBuffer*, uint64_t>
			{
				Block& block = GetBlock(requested_size);
				block.used += requested_size;
				return { block.buffer, block.used - requested_size };
			}

			auto Reset() -> void
			{
				for (auto& block : blocks)
					block.used = 0;
			}
		private:
			auto GetBlock(uint64_t requested_size) -> Block&
			{
				auto found_block = std::find_if(blocks.begin(), blocks.end(), [requested_size](const Block& block) {
					return  block.buffer->GetSize() - block.used > requested_size;
					});

				if (found_block != blocks.end())
					return *found_block;

				return AddBlock(std::max(block_size, requested_size));
			}

			auto AddBlock(uint64_t size) -> Block&
			{
				BufferCreateInfo buffer_ci{};
				buffer_ci.size = size;
				buffer_ci.usage = BufferUsageFlag::eStagingBuffer;

				auto& block = blocks.emplace_back(Block{
					.buffer = gfx_context->CreateBuffer(buffer_ci),
					.used = 0ull
					});

				block.buffer->SetResourceName("uploader_staging");

				return block;
			}
		private:
			IContext* gfx_context{ nullptr };

			uint64_t block_size{ 0ull };
			std::vector<Block> blocks{};
		};

		class MipGenerator
		{
		public:
			MipGenerator(IContext* gfx_context) :
				gfx_context{ gfx_context }
			{
				const std::span<const uint8_t> mip_generate_shaders[] =
				{
					{ reinterpret_cast<const uint8_t*>(MipGeneratorShader), MipGeneratorShader_size },
					{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip2), MipGeneratorShaderMip2_size },
					{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip3), MipGeneratorShaderMip3_size },
					{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip4), MipGeneratorShaderMip4_size }
				};
		
				// Load all shaders
				for (int32_t i = 0; i < shader_instances.size(); ++i)
				{
					auto& shader_instance = shader_instances[i];
		
					const edge::gfx::ShaderStageInfo shader_stages[]
					{
						{
							.stage = edge::gfx::ShaderStageFlag::eCompute,
							.code = mip_generate_shaders[i]
						}
					};
		
					shader_instance.shader = gfx_context->CreateShader(shader_stages);
		
					RootSignatureCreateInfo root_sign_ci{};
					root_sign_ci.shader = shader_instance.shader;
		
					shader_instance.root_signature = gfx_context->CreateRootSignature(root_sign_ci);
		
					ComputePipelineCreateInfo pipeline_ci{};
					pipeline_ci.shader = shader_instance.shader;
					pipeline_ci.root_signature = shader_instance.root_signature;
		
					shader_instance.pipeline = gfx_context->CreatePipeline(pipeline_ci);
				}
		
				// Create descriptor allocator for allocate descriptors in mip passes
				descriptor_allocator = gfx_context->CreateDescriptorAllocator();
		
				// Also we need linear sampler 
		
				SamplerCreateInfo sampler_ci{};
				sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = SamplerAddressMode::eClampToEdge;
				sampler_ci.min_filter = Filter::eLinear;
				sampler_ci.mag_filter = Filter::eLinear;
				sampler_ci.mip_map_mode = Filter::eLinear;
		
				linear_sampler = gfx_context->CreateSampler(sampler_ci);
			}
		
			~MipGenerator()
			{
				// Delete mip generation resources
				for (int32_t i = 0; i < shader_instances.size(); ++i)
				{
					gfx_context->Destroy(shader_instances[i].pipeline);
					gfx_context->Destroy(shader_instances[i].root_signature);
					gfx_context->Destroy(shader_instances[i].shader);
				}
		
				gfx_context->Destroy(descriptor_allocator);
			}
		
			ResourceStateFlags WriteCommands(ICommandList* cmd_list, ITexture* texture)
			{
				// ITexture should be in copy dest state
				ResourceStateFlags resource_state{ ResourceStateFlag::eCopyDst };
		
				TextureBarrier texture_barrier{};
				texture_barrier.texture = texture;
		
				ResourceBarrierInfo resource_barrier{};
				resource_barrier.texture_barriers = &texture_barrier;
				resource_barrier.texture_barrier_count = 1u;
		
				uint32_t total_mip_count = texture->GetMipLevelCount();
				for (int32_t mip = 1; mip < total_mip_count;)
				{
					uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };
		
					group_x = texture->GetWidth() >> mip;
					group_y = texture->GetHeight() >> mip;
		
					uint32_t mip_count{ 4u };
					if (mip + mip_count > total_mip_count)
						mip_count = total_mip_count - mip;
		
					auto& generator = shader_instances[mip_count - 1];
		
					IResourceDescriptor* descriptor = descriptor_allocator->AllocateDescriptors(generator.root_signature);
		
					cmd_list->BindPipeline(generator.pipeline);
		
					// At first make texture readable
					texture_barrier.src_state = resource_state;
					texture_barrier.dst_state = ResourceStateFlag::eRead | ResourceStateFlag::eCompute | ResourceStateFlag::eSample | ResourceStateFlag::eGeneral;
					cmd_list->ResourceBarrier(resource_barrier);
					resource_state = texture_barrier.dst_state;
		
					TextureDescriptor bind_resource{};
					bind_resource.type = DescriptorType::eCombinedImageSampler;
					bind_resource.set = 0;
					bind_resource.binding = 5;
		
					bind_resource.texture = texture;
					bind_resource.sampler = linear_sampler;
					bind_resource.subresource_index = mip - 1;
					bind_resource.resource_state = resource_state;
		
					// Bind texture slot
					descriptor->BindTexture(bind_resource);
		
					// Make texture writable now
					texture_barrier.src_state = resource_state;
					texture_barrier.dst_state = ResourceStateFlag::eWrite | ResourceStateFlag::eCompute | ResourceStateFlag::eStorage | ResourceStateFlag::eGeneral;
					cmd_list->ResourceBarrier(resource_barrier);
					resource_state = texture_barrier.dst_state;
		
					bind_resource.type = DescriptorType::eStorageImage;
					bind_resource.resource_state = resource_state;
		
					for (int32_t i = 0; i < mip_count; ++i)
					{
						bind_resource.binding = i + 1;
						bind_resource.subresource_index = mip + i;
						descriptor->BindTexture(bind_resource);
					}
		
					// Finally bind descriptor
					BindDescriptorsInfo bind_info{};
					bind_info.pipeline_type = PipelineType::eCompute;
					bind_info.resource_descriptors = { &descriptor, 1u };
					bind_info.root_signature = generator.root_signature;
		
					descriptor->Flush();
					cmd_list->BindDescriptors(bind_info);
		
					// Divide by local group sizes
					auto& reflection = generator.shader->GetReflection();
					group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
					group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
					group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);
		
					cmd_list->Dispatch(group_x, group_y, group_z);
		
					mip += mip_count;
				}
		
				return resource_state;
			}
		
			void Reset()
			{
				descriptor_allocator->Reset();
			}
		private:
			IContext* gfx_context{ nullptr };
		
			struct ShaderInstance
			{
				IShader* shader{ nullptr };
				IRootSignature* root_signature{ nullptr };
				IPipeline* pipeline{ nullptr };
			};
		
			ISampler* linear_sampler{ nullptr };
		
			IDescriptorAllocator* descriptor_allocator{ nullptr };
		
			std::array<ShaderInstance, 4ull> shader_instances;
		};

		class ITextureReader
		{
		public:
			virtual ~ITextureReader() = default;

			virtual auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool = 0;
			virtual auto IsNeedToGenerateMips() const noexcept -> bool = 0;
			virtual auto GetSize() -> uint64_t = 0;
			virtual auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void = 0;
		private:
		};

		class STBImageLoader final : public ITextureReader
		{
		public:
			~STBImageLoader() override
			{
				if (image_data)
				{
					stbi_image_free(image_data);
				}
			}

			auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool override
			{
				image_data = stbi_load_from_memory(data.data(), data.size(), &image_width, &image_height, &image_channel_count, STBI_rgb_alpha);

				create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eTransferable;
				create_info.width = image_width;
				create_info.height = image_height;
				//create_info.format = image_channel_count == 4 ? gfx::Format::eR8G8B8A8Unorm : gfx::Format::eR8G8B8Unorm;
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(image_width, image_height)))) + 1u;

				return image_data;
			}

			auto IsNeedToGenerateMips() const noexcept -> bool override
			{
				return true;
			}

			auto GetSize() -> uint64_t override
			{
				return image_width * image_height * 4ull;
			}

			auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void override
			{
				TextureCopyRegion copy_region{};
				copy_region.buffer_offset = src_offset;
				copy_region.size_x = image_width;
				copy_region.size_y = image_height;

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = dst_texture;
				copy_texture.copy_regions = { &copy_region, 1 };

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, image_data, GetSize());

				cmd_list->CopyTexture(copy_texture);
			}
		private:
			uint8_t* image_data{ nullptr };
			int32_t image_width{ 1 };
			int32_t image_height{ 1 };
			int32_t image_channel_count{ 1 };
		};

		class KTXImageLoader final : public ITextureReader
		{
		public:
			~KTXImageLoader() override
			{
				if (texture)
				{
					ktxTexture_Destroy(texture);
				}
			}

			auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool override
			{
				auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
				assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
				if (ktxresult != KTX_SUCCESS)
					return false;

				if (texture->isCubemap)
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;

				create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;
				create_info.width = texture->baseWidth;
				create_info.height = texture->baseHeight;
				create_info.depth = texture->baseDepth;

				create_info.levels = texture->numLevels;
				create_info.layers = texture->numLayers;

				if (texture->generateMipmaps)
				{
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;
					create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
				}

				switch (texture->classId)
				{
				case ktxTexture1_c: {
					ktxTexture1* ktxtexture1 = (ktxTexture1*)texture;

					//uint32_t vk_format = vkGetFormatFromOpenGLInternalFormat(ktxtexture1->glInternalformat);
					//if (vk_format == 0u)
					//{
					//	vk_format = vkGetFormatFromOpenGLFormat(ktxtexture1->glFormat, ktxtexture1->glType);
					//}
					//
					//create_info.format = gfx::FormatFromVk(vk_format);
				} break;
				case ktxTexture2_c: {
					ktxTexture2* ktxtexture2 = (ktxTexture2*)texture;

					if (ktxTexture2_NeedsTranscoding(ktxtexture2))
					{
						ktxresult = ktxTexture2_TranscodeBasis(ktxtexture2, KTX_TTF_BC7_RGBA, 0);
					}
						
					create_info.format = gfx::FormatFromVk(ktxtexture2->vkFormat);
				} break;
				}

				return true;
			}

			auto IsNeedToGenerateMips() const noexcept -> bool override
			{
				return texture->generateMipmaps;
			}

			auto GetSize() -> uint64_t override
			{
				return texture->dataSize;
			}

			auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void override
			{
				std::vector<TextureCopyRegion> copy_regions{};

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = dst_texture;

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, texture->pData, texture->dataSize);

				// Collect image copy regions
				auto num_layers = texture->isCubemap ? texture->numFaces : texture->numLayers;
				for (int32_t layer = 0; layer < num_layers; layer++)
				{
					for (int32_t level = 0; level < texture->numLevels; level++)
					{
						auto& region = copy_regions.emplace_back(src_offset, 0, 0, 0, 
							(texture->baseWidth >> level), 
							(texture->baseHeight >> level), 1, 
							layer, texture->numLayers, level);

						auto ktxresult = ktxTexture_GetImageOffset(texture, level, 0, layer, &region.buffer_offset);
						if (ktxresult != KTX_SUCCESS)
							return;
					}
				}

				copy_texture.copy_regions = copy_regions;
				cmd_list->CopyTexture(copy_texture);
			}
		protected:
			ktxTexture* texture{ nullptr };
		};

		IContext* ResourceUpdater::gfx_context{ nullptr };
		fs::VirtualFilesystem* ResourceUpdater::filesystem{ nullptr };
		const IQueue* ResourceUpdater::queue{ nullptr };
		std::vector<ResourceUpdater::Node> ResourceUpdater::nodes{};
		uint32_t ResourceUpdater::active_node_idx{ 0u };
		bool ResourceUpdater::recording{ false };
		std::unique_ptr<MipGenerator> ResourceUpdater::mip_generator{};

		struct ResourceUpdater::Node
		{
			Node(IContext* gfx_context, uint64_t size, std::string_view name) :
				allocator{ gfx_context, size }
			{
				fence = gfx_context->CreateFence();
				fence->SetResourceName(name);
				semaphore = gfx_context->CreateSemaphore();
				semaphore->SetResourceName(name);
				cmd_pool = gfx_context->CreateCommandPool(queue);
				cmd_pool->SetResourceName(name);
			}

			IFence* fence;
			ISemaphore* semaphore;

			ICommandList* cmd_list{ nullptr };
			ICommandPool* cmd_pool;

			BlockAllocator allocator;
		};


		void ResourceUpdater::Create(const ResourceUpdaterCreateInfo& create_info)
		{
			ResourceUpdater::gfx_context = create_info.gfx_context;
			ResourceUpdater::filesystem = create_info.filesystem;

			queue = gfx_context->RequestQueue(QueueTypeFlag::eGraphics);

			for (int32_t i = 0; i < create_info.buffer_count; ++i)
			{
				auto debug_name = std::format("updater_{}", i);
				nodes.emplace_back(gfx_context, create_info.buffer_size, debug_name);
			}

			mip_generator = std::make_unique<MipGenerator>(gfx_context);
		}

		void ResourceUpdater::Shutdown()
		{
			for (auto& node : nodes)
			{
				gfx_context->Destroy(node.fence);
				gfx_context->Destroy(node.semaphore);
				gfx_context->Destroy(node.cmd_pool);
			}

			mip_generator = nullptr;

			nodes.clear();
		}

		ICommandList* ResourceUpdater::AcquireCommandList()
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& node = nodes[active_node_idx];
			if (!recording)
			{
				node.fence->Wait();
				node.fence->Reset();

				node.allocator.Reset();
				node.cmd_pool->Reset();
				node.cmd_list = nullptr;

				node.cmd_list = node.cmd_pool->Allocate();
				node.cmd_list->Begin();
				node.cmd_list->BeginMarker("UPDATER", 0xFFFF00FF);
				recording = true;
			}
			return node.cmd_list;
		}

		auto ResourceUpdater::LoadTexture(std::string_view path) -> ITexture*
		{
			EDGE_PROFILE_ZONE_SCOPED;

			ICommandList* cmd_list = AcquireCommandList();

			auto ext = edge::fsh::get_ext(path);
			auto ext_hash = murmur_hash(ext.data(), ext.size(), 0);

			std::unique_ptr<ITextureReader> texture_reader;

			switch (ext_hash)
			{
			case edge::fsh::format::ktx:
			case edge::fsh::format::ktx2: { texture_reader = std::make_unique<KTXImageLoader>(); } break;
			default: { texture_reader = std::make_unique<STBImageLoader>(); } break;
			}

			std::vector<uint8_t> data{};
			filesystem->ReadFile(path, data);

			gfx::TextureCreateInfo create_info{};
			texture_reader->Load(data, create_info);

			ITexture* dst_texture = gfx_context->CreateTexture(create_info);

			auto& node = nodes[active_node_idx];
			auto [src_buffer, src_offset] = node.allocator.Allocate(texture_reader->GetSize());

			TextureBarrier texture_barrier{};
			texture_barrier.texture = dst_texture;
			texture_barrier.src_state = ResourceStateFlag::eUndefined;
			texture_barrier.dst_state = ResourceStateFlag::eCopyDst;
			
			ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;

			// Transfer to intermediate layout
			cmd_list->ResourceBarrier(resource_barrier);

			texture_reader->Upload(cmd_list, src_buffer, src_offset, dst_texture);

			if (texture_reader->IsNeedToGenerateMips())
				texture_barrier.src_state = mip_generator->WriteCommands(cmd_list, dst_texture);
			else
				texture_barrier.src_state = ResourceStateFlag::eCopyDst;

			// Transfer to final layout
			texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;
			cmd_list->ResourceBarrier(resource_barrier);

			return dst_texture;
		}

		void ResourceUpdater::UpdateTexture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			ICommandList* cmd_list = AcquireCommandList();

			auto& node = nodes[active_node_idx];
			auto [src_buffer, src_offset] = node.allocator.Allocate(data.size());

			auto* src_memory = src_buffer->Map();
			memcpy(src_memory + src_offset, data.data(), data.size());

			TextureCopyRegion copy_region{};
			copy_region.buffer_offset = src_offset;
			copy_region.size_x = dst_texture->GetWidth();
			copy_region.size_y = dst_texture->GetHeight();
			copy_region.size_z = dst_texture->GetDepth();

			CopyTextureInfo copy_texture{};
			copy_texture.dst_texture = dst_texture;
			copy_texture.src_buffer = src_buffer;
			copy_texture.copy_regions = { &copy_region, 1 };
			
			TextureBarrier texture_barrier{};
			texture_barrier.texture = dst_texture;
			texture_barrier.src_state = current_state;
			current_state = texture_barrier.dst_state = ResourceStateFlag::eCopyDst;
			
			ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;
			
			// Transfer to intermediate layout
			cmd_list->ResourceBarrier(resource_barrier);
			
			// Copy texture regions
			cmd_list->CopyTexture(copy_texture);
			
			// Transfer to final layout
			texture_barrier.src_state = current_state;
			current_state = texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;
			cmd_list->ResourceBarrier(resource_barrier);
		}

		void ResourceUpdater::Flush(SemaphoreSubmitInfo& signalled_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!recording)
				return;

			auto& node = nodes[active_node_idx];
			node.cmd_list->EndMarker();
			node.cmd_list->End();

			signalled_semaphore.semaphore = node.semaphore;
			signalled_semaphore.stage = StageFlag::eTransfer;

			SubmitInfo submit_info{};
			submit_info.command_lists = { &node.cmd_list, 1 };
			submit_info.signal_semaphore_infos = { &signalled_semaphore, 1 };
			submit_info.wait_semaphore_infos = wait_semaphores;

			queue->Submit(submit_info, node.fence);

			recording = false;

			active_node_idx = (active_node_idx + 1) % nodes.size();
		}

		auto ResourceUpdater::GetBufferUpdater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater
		{
			AcquireCommandList();

			auto& node = nodes[active_node_idx];
			auto [src_buffer, src_offset] = node.allocator.Allocate(update_size);

			return { src_buffer, src_offset, update_size, dst_buffer };
		}

		void ResourceUpdater::SubmitBufferUpdater(BufferUpdater& updater, ResourceStateFlags& current_state)
		{
			ICommandList* cmd_list = AcquireCommandList();
			updater.Submit(cmd_list, current_state);
		}

		void ResourceUpdater::UpdateBuffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, std::span<const uint8_t> data)
		{
			auto updater = GetBufferUpdater(dst_buffer, data.size());
			updater.AddRegion(data);
			SubmitBufferUpdater(updater, current_state);
		}



		IContext* ResourceUploader::gfx_context{ nullptr };
		fs::VirtualFilesystem* ResourceUploader::filesystem{ nullptr };
		const IQueue* ResourceUploader::queue{ nullptr };
		std::vector<ResourceUploader::Node> ResourceUploader::nodes{};
		uint32_t ResourceUploader::active_node_idx{ 0u };
		std::unique_ptr<MipGenerator> ResourceUploader::mip_generator{};
		std::jthread ResourceUploader::upload_thread{};
		std::stop_source ResourceUploader::uploader_stop{};
		bool ResourceUploader::recording{ false };


		struct ResourceUploader::Node
		{
			Node(IContext* gfx_context, uint64_t size, std::string_view name) :
				allocator{ gfx_context, size }
			{
				fence = gfx_context->CreateFence();
				fence->SetResourceName(name);
				semaphore = gfx_context->CreateSemaphore();
				semaphore->SetResourceName(name);
				cmd_pool = gfx_context->CreateCommandPool(queue);
				cmd_pool->SetResourceName(name);
			}

			IFence* fence;
			ISemaphore* semaphore;

			ICommandList* cmd_list{ nullptr };
			ICommandPool* cmd_pool;

			BlockAllocator allocator;
		};


		void ResourceUploader::Create(const ResourceUpdaterCreateInfo& create_info)
		{
			ResourceUploader::gfx_context = create_info.gfx_context;
			ResourceUploader::filesystem = create_info.filesystem;

			queue = gfx_context->RequestQueue(QueueTypeFlag::eGraphics);

			for (int32_t i = 0; i < create_info.buffer_count; ++i)
			{
				auto debug_name = std::format("uploader_{}", i);
				nodes.emplace_back(gfx_context, create_info.buffer_size, debug_name);
			}

			mip_generator = std::make_unique<MipGenerator>(gfx_context);

			upload_thread = std::jthread(ResourceUploader::UploaderThread);
			uploader_stop = upload_thread.get_stop_source();
			upload_thread.detach();
		}

		void ResourceUploader::Shutdown()
		{
			if (uploader_stop.stop_possible())
			{
				uploader_stop.stop_requested();
			}

			for (auto& node : nodes)
			{
				gfx_context->Destroy(node.fence);
				gfx_context->Destroy(node.semaphore);
				gfx_context->Destroy(node.cmd_pool);
			}

			mip_generator = nullptr;

			nodes.clear();
		}

		void ResourceUploader::UploaderThread(std::stop_token stop_token)
		{
			while (!stop_token.stop_requested())
			{

			}
		}




		// Buffer updater implementation
		BufferUpdater::BufferUpdater(IBuffer* src_buffer, uint64_t src_offset, uint64_t src_size, IBuffer* dst_buffer) :
			copy_buffer_info{ src_buffer, dst_buffer, {} },
			src_region_size{ src_size },
			src_offset{ src_offset }
		{
			src_memory = src_buffer->Map();
		}

		void BufferUpdater::AddRegion(std::span<const uint8_t> data)
		{
			assert(data.size() <= src_region_size && (dst_offset + data.size() <= copy_buffer_info.dst_buffer->GetSize()) && "Trying to add region out of range.");

			copy_regions.push_back({
					src_offset,
					dst_offset,
					data.size()
				});

			memcpy(src_memory + src_offset, data.data(), data.size());

			src_offset += data.size();
			dst_offset += data.size();
		}

		void BufferUpdater::InsertRegion(std::span<const uint8_t> data, uint64_t offset)
		{
			assert(data.size() <= src_region_size && (offset + data.size() <= copy_buffer_info.dst_buffer->GetSize()) && "Trying to insert region out of range.");

			copy_regions.push_back({
					src_offset,
					offset,
					data.size()
				});

			//engine::util::Copy(src_memory, src_offset, data.data(), data.size(), offset);

			memcpy(src_memory + src_offset, data.data(), data.size());

			src_offset += data.size();
		}

		void BufferUpdater::Submit(ICommandList* cmd_list, ResourceStateFlags& current_state)
		{
			copy_buffer_info.src_buffer->Unmap();

			// Sort regions by destination offset
			std::sort(copy_regions.begin(), copy_regions.end(),
				[](const BufferCopyRegion& a, const BufferCopyRegion& b) {
					return a.dst_offset < b.dst_offset;
				});

			std::vector<BufferCopyRegion> merged_regions{};
			merged_regions.emplace_back(copy_regions.front());

			for (size_t i = 1; i < copy_regions.size(); ++i)
			{
				BufferCopyRegion& last = merged_regions.back();
				const BufferCopyRegion& current = copy_regions[i];

				uint64_t last_end = last.dst_offset + last.size;
				uint64_t current_end = current.dst_offset + current.size;

				// Check if regions are contiguous or overlapping
				if (last_end >= current.dst_offset &&
					(last.src_offset + (current.dst_offset - last.dst_offset) == current.src_offset))
				{
					// Merge regions by expanding the last region
					last.size = std::max(last_end, current_end) - last.dst_offset;
				}
				else
				{
					// Regions can't be merged; add a new region
					merged_regions.push_back(current);
				}
			}

			if (current_state != ResourceStateFlag::eCopyDst)
			{
				// Translate dst_buffer for correct layout
				BufferBarrier buffer_barrier{};
				buffer_barrier.buffer = copy_buffer_info.dst_buffer;
				buffer_barrier.src_state = current_state;
				current_state = buffer_barrier.dst_state = ResourceStateFlag::eCopyDst;
				buffer_barrier.range.size = src_region_size;

				ResourceBarrierInfo resource_barrier{};
				resource_barrier.buffer_barriers = &buffer_barrier;
				resource_barrier.buffer_barrier_count = 1;

				// TODO: Make resource translations only if needed
				cmd_list->ResourceBarrier(resource_barrier);
			}

			copy_buffer_info.regions = merged_regions;
			cmd_list->CopyBuffer(copy_buffer_info);
		}
	}
}
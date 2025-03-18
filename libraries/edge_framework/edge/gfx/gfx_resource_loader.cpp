#include "gfx_resource_loader.h"
#include "gfx_context.h"

#include "../fs/fs_helper.h"
#include "../util/Logger.h"
#include "../util/Profiler.h"
#include "../util/util.h"
#include "../util/Memory.hpp"

#include "../fs/fs_virtual_filesystem.h"

#include <format>

#include "MipGeneratorShader.h"
#include "MipGeneratorShaderMip2.h"
#include "MipGeneratorShaderMip3.h"
#include "MipGeneratorShaderMip4.h"

#include "resource_loader/gfx_staging_allocator.h"
#include "resource_loader/gfx_resource_uploader.h"
#include "resource_loader/gfx_texture_loader.h"

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

namespace edge
{
	namespace gfx
	{
		struct CommandNode
		{
			CommandNode(IContext* gfx_context, const IQueue* queue, std::string_view name) :
				gfx_context{ gfx_context },
				queue{ queue }
			{
				fence = gfx_context->CreateFence();
				fence->SetResourceName(name);
				semaphore = gfx_context->CreateTimelineSemaphore();
				semaphore->SetResourceName(name);
				cmd_pool = gfx_context->CreateCommandPool(queue);
				cmd_pool->SetResourceName(name);
			}

			virtual ~CommandNode()
			{
				gfx_context->Destroy(fence);
				gfx_context->Destroy(semaphore);
				gfx_context->Destroy(cmd_pool);
			}

			CommandNode(CommandNode&& other) noexcept :
				gfx_context{ std::exchange(other.gfx_context, nullptr) },
				fence{ std::exchange(other.fence, nullptr) },
				semaphore{ std::exchange(other.semaphore, nullptr) },
				cmd_list{ std::exchange(other.cmd_list, nullptr) },
				cmd_pool{ std::exchange(other.cmd_pool, nullptr) }
			{
			}

			auto operator=(CommandNode&& other) noexcept -> CommandNode&
			{
				gfx_context = std::exchange(other.gfx_context, nullptr);
				fence = std::exchange(other.fence, nullptr);
				semaphore = std::exchange(other.semaphore, nullptr);
				cmd_list = std::exchange(other.cmd_list, nullptr);
				cmd_pool = std::exchange(other.cmd_pool, nullptr);

				return*this;
			}

			CommandNode(const CommandNode&) = delete;
			CommandNode& operator=(const CommandNode&) = delete;

			[[nodiscard]] auto AcquireCommandList(bool& recording, std::string_view name) -> ICommandList*
			{
				EDGE_PROFILE_ZONE_SCOPED;

				if (!recording)
				{
					fence->Wait();
					fence->Reset();

					cmd_pool->Reset();
					cmd_list = nullptr;

					cmd_list = cmd_pool->Allocate();
					cmd_list->Begin();
					cmd_list->BeginMarker(name, 0xFFFF00FF);
					recording = true;
				}
				return cmd_list;
			}

			auto Flush(bool& recording, SemaphoreSubmitInfo& signalled_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores) -> void
			{
				EDGE_PROFILE_ZONE_SCOPED;

				if (!recording)
					return;

				cmd_list->EndMarker();
				cmd_list->End();

				signalled_semaphore.semaphore = semaphore;
				signalled_semaphore.value = ++counter;

				SubmitInfo submit_info{};
				submit_info.command_lists = { &cmd_list, 1 };
				submit_info.signal_semaphore_infos = { &signalled_semaphore, 1 };
				submit_info.wait_semaphore_infos = wait_semaphores;
				queue->Submit(submit_info, fence);

				recording = false;
			}

			IContext* gfx_context;
			const IQueue* queue;

			IFence* fence;
			ISemaphore* semaphore;

			ICommandList* cmd_list{ nullptr };
			ICommandPool* cmd_pool;

			uint64_t counter{ 0 };
		};

		struct UploaderNode : CommandNode
		{
			UploaderNode(IContext* gfx_context, const IQueue* queue, uint64_t size, std::string_view name) :
				CommandNode{ gfx_context, queue, name }
			{
				allocator = std::make_unique<ArenaStagingAllocator>(gfx_context, size);
			}

			UploaderNode(UploaderNode&& other) noexcept :
				CommandNode{ std::move(other) },
				allocator{ std::move(other.allocator) }
			{
			}

			auto operator=(UploaderNode&& other) noexcept -> UploaderNode&
			{
				CommandNode::operator=(std::move(other));
				allocator = std::move(other.allocator);

				return*this;
			}

			UploaderNode(const UploaderNode&) = delete;
			UploaderNode& operator=(const UploaderNode&) = delete;

			std::unique_ptr<StagingBufferAllocator> allocator;
		};

		class CommandExecutor
		{
		public:
			CommandExecutor(IContext* gfx_context, const IQueue* queue, StageFlag stage, std::string_view name) :
				gfx_context{ gfx_context },
				queue{ queue },
				stage{ stage },
				name{ name }
			{
			}

			auto Flush(SemaphoreSubmitInfo& signalled_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores) -> void
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto& node = GetActiveNode();
				signalled_semaphore.stage = stage;
				node.Flush(recording, signalled_semaphore, wait_semaphores);
				active_node_index = (active_node_index + 1) % nodes.size();
			}

			[[nodiscard]] virtual auto AcquireCommandList() -> ICommandList*
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto& node = GetActiveNode();
				return node.AcquireCommandList(recording, name);
			}

			auto GetActiveNode() -> CommandNode&
			{
				return *nodes[active_node_index];
			}
		protected:
			IContext* gfx_context;
			const IQueue* queue;
			std::vector<std::unique_ptr<CommandNode>> nodes;
			uint32_t active_node_index{ 0 };
			StageFlag stage;
			std::string_view name;
			bool recording{ false };
		};

		class MipGenerator : public CommandExecutor
		{
		public:
			MipGenerator(IContext* gfx_context, const IQueue* queue, uint32_t generator_count) :
				CommandExecutor{ gfx_context, queue, StageFlag::eComputeShader, "MIP_GENERATOR" }
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

				for (int32_t i = 0; i < generator_count; ++i)
				{
					auto debug_name = std::format("MIP_GENERAOTR_{}", i);
					nodes.emplace_back(std::make_unique<CommandNode>(gfx_context, queue, debug_name));
				}
		
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
		
			ResourceStateFlags WriteCommands(ITexture* texture, std::string_view path)
			{
				ICommandList* cmd_list = AcquireCommandList();
				cmd_list->BeginMarker(path, 0xFFFFFFFF);

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

				cmd_list->EndMarker();
		
				return resource_state;
			}
		
			void Reset()
			{
				descriptor_allocator->Reset();
			}
		private:
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

				cmd_list->ResourceBarrier(resource_barrier);
			}

			copy_buffer_info.regions = merged_regions;
			cmd_list->CopyBuffer(copy_buffer_info);
		}

		class ResourceLoader final : public CommandExecutor
		{
		public:
			ResourceLoader(IContext* gfx_context, const IQueue* queue, uint32_t node_count, uint64_t buffer_size, std::string_view name = {}) :
				CommandExecutor{ gfx_context, queue, StageFlag::eTransfer, name }
			{
				for (int32_t i = 0; i < node_count; ++i)
				{
					auto debug_name = std::format("{}_{}", name, i);
					nodes.emplace_back(std::make_unique<UploaderNode>(gfx_context, queue, buffer_size, debug_name));
				}
			}

			[[nodiscard]] auto AcquireCommandList() -> ICommandList* override
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto& node = (UploaderNode&)GetActiveNode();
				if (!recording)
				{
					node.allocator->Reset();
				}

				return node.AcquireCommandList(recording, name);
			}
		};

		IContext* ResourceUpdater::gfx_context{ nullptr };
		fs::VirtualFilesystem* ResourceUpdater::filesystem{ nullptr };
		std::unique_ptr<CommandExecutor> ResourceUpdater::updater{};
		std::unique_ptr<UploadingDispatcher> ResourceUpdater::uploading_dispatcher{};

		void ResourceUpdater::Create(const ResourceUpdaterCreateInfo& create_info)
		{
			ResourceUpdater::gfx_context = create_info.gfx_context;
			ResourceUpdater::filesystem = create_info.filesystem;

			auto update_queue = gfx_context->RequestQueue(QueueTypeFlag::eGraphics);
			update_queue->SetResourceName("UPDATE");
			updater = std::make_unique<ResourceLoader>(gfx_context, update_queue, create_info.buffer_count, create_info.buffer_size, "UPDATER");

			//auto compute_queue = gfx_context->RequestQueue(QueueTypeFlag::eCompute, QueueTypeFlag::eGraphics);
			//compute_queue->SetResourceName("COMPUTE_ASYNC");
			//mip_generator = std::make_unique<MipGenerator>(gfx_context, compute_queue, create_info.buffer_count);

			uploading_dispatcher = std::make_unique<UploadingDispatcher>(gfx_context, create_info.buffer_size, 8);
		}

		void ResourceUpdater::Shutdown()
		{
			updater = nullptr;
			uploading_dispatcher = nullptr;
			//mip_generator = nullptr;
		}

		auto ResourceUpdater::LoadTexture(std::string path) -> std::shared_ptr<AsyncResourceHandle>
		{
			std::packaged_task<ResourceUploadFnSign> task([path](ICommandList* cmd_list, StagingBufferAllocator& allocator) -> ResourceUploadResult
				{
					EDGE_PROFILE_ZONE_SCOPED;

					ResourceStateFlags state{};
					return std::make_pair(LoadTextureImpl(cmd_list, allocator, path, state), state);
				});

			auto handle = std::make_shared<AsyncResourceHandle>();
			uploading_dispatcher->Submit({ handle, std::move(task) });

			return handle;
		}

		auto ResourceUpdater::LoadTextureSync(std::string path, ResourceStateFlags& state) -> ITexture*
		{
			EDGE_PROFILE_ZONE_SCOPED;

			ICommandList* cmd_list = updater->AcquireCommandList();
			auto& node = (UploaderNode&)updater->GetActiveNode();
			return LoadTextureImpl(cmd_list, *node.allocator, path, state);
		}

		auto ResourceUpdater::SyncUploads() -> void
		{
			//needs_sync.store(true, std::memory_order_relaxed);
		}

		void ResourceUpdater::UpdateTexture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			ICommandList* cmd_list = updater->AcquireCommandList();

			auto& node = (UploaderNode&)updater->GetActiveNode();
			auto [src_buffer, src_offset] = node.allocator->Allocate(data.size());

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

		bool ResourceUpdater::Flush(std::vector<SemaphoreSubmitInfo>& signalled_semaphores, std::span<SemaphoreSubmitInfo> wait_semaphores)
		{
			SemaphoreSubmitInfo updater_semaphore{};
			updater->Flush(updater_semaphore, wait_semaphores);
			if (updater_semaphore.semaphore)
				signalled_semaphores.emplace_back(updater_semaphore);

			bool uploads_flushed{ false };
			SemaphoreSubmitInfo uploader_semaphore{};
			if (uploading_dispatcher->IsSyncNeeded(uploader_semaphore))
			{
				signalled_semaphores.emplace_back(uploader_semaphore);
				uploads_flushed = true;
			}

			return uploads_flushed;
		}

		auto ResourceUpdater::GetBufferUpdater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater
		{
			updater->AcquireCommandList();

			auto& node = (UploaderNode&)updater->GetActiveNode();
			auto [src_buffer, src_offset] = node.allocator->Allocate(update_size);

			return { src_buffer, src_offset, update_size, dst_buffer };
		}

		void ResourceUpdater::SubmitBufferUpdater(BufferUpdater& buffer_updater, ResourceStateFlags& current_state)
		{
			ICommandList* cmd_list = updater->AcquireCommandList();
			buffer_updater.Submit(cmd_list, current_state);
		}

		void ResourceUpdater::UpdateBuffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, std::span<const uint8_t> data)
		{
			auto updater = GetBufferUpdater(dst_buffer, data.size());
			updater.AddRegion(data);
			SubmitBufferUpdater(updater, current_state);
		}

		auto ResourceUpdater::LoadTextureImpl(ICommandList* cmd_list, StagingBufferAllocator& allocator, std::string path, ResourceStateFlags& output_state) -> ITexture*
		{
			EDGE_LOGD("[ResourceUpdater] Loading texture \"{}\"", path);
			cmd_list->BeginMarker(path, 0xFFFFFFFF);

			std::vector<uint8_t> data{};
			filesystem->ReadFile(path, data);

			auto texture_loader = MakeTextureLoader(path);

			gfx::TextureCreateInfo create_info{};
			texture_loader->Load(data, create_info);

			ITexture* dst_texture = gfx_context->CreateTexture(create_info);
			dst_texture->SetResourceName(path);

			auto block_align = GetFormatBlockAlign(create_info.format);
			auto [src_buffer, src_offset] = allocator.Allocate(texture_loader->GetSize(), block_align);

			TextureBarrier texture_barrier{};
			texture_barrier.texture = dst_texture;
			texture_barrier.src_state = ResourceStateFlag::eUndefined;
			texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

			ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = &texture_barrier;
			resource_barrier.texture_barrier_count = 1u;

			// Transfer to intermediate layout
			cmd_list->ResourceBarrier(resource_barrier);

			texture_loader->Upload(cmd_list, src_buffer, src_offset, dst_texture);

			cmd_list->EndMarker();

			output_state = ResourceStateFlag::eCopyDst;

			EDGE_LOGD("[ResourceUpdater] Loading texture \"{}\" done", path);

			//if (texture_reader->IsNeedToGenerateMips())
			//	resource_result.state = mip_generator->WriteCommands(dst_texture, path);

			return dst_texture;
		}
	}
}
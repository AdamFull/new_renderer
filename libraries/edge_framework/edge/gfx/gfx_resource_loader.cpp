#include "gfx_resource_loader.h"
#include "gfx_context.h"

#include "../fs/fs_helper.h"
#include "../util/Logger.h"
#include "../util/utl_profiler.h"
#include "../util/utl_base.h"
#include "../util/utl_memory.hpp"

#include "../fs/fs_virtual_filesystem.h"

#include <format>

#include "MipGeneratorShader.h"
#include "MipGeneratorShaderMip2.h"
#include "MipGeneratorShaderMip3.h"
#include "MipGeneratorShaderMip4.h"

#include "resource_loader/gfx_staging_allocator.h"
#include "resource_loader/gfx_resource_uploader.h"
#include "resource_loader/gfx_texture_loader.h"

namespace edge::gfx
{
	struct CommandNode {
		CommandNode(const IQueue* queue, std::string_view name) :
			queue{ queue } {

			fence = create_fence();
			fence->set_resource_name(name);
			semaphore = create_semaphore(SemaphoreType::eTimeline);
			semaphore->set_resource_name(name);
			cmd_pool = create_command_pool(queue);
			cmd_pool->set_resource_name(name);
		}

		virtual ~CommandNode() {
			destroy(fence);
			destroy(semaphore);
			destroy(cmd_pool);
		}

		CommandNode(CommandNode&& other) noexcept :
			fence{ std::exchange(other.fence, nullptr) },
			semaphore{ std::exchange(other.semaphore, nullptr) },
			cmd_list{ std::exchange(other.cmd_list, nullptr) },
			cmd_pool{ std::exchange(other.cmd_pool, nullptr) } { }

		auto operator=(CommandNode&& other) noexcept -> CommandNode& {
			fence = std::exchange(other.fence, nullptr);
			semaphore = std::exchange(other.semaphore, nullptr);
			cmd_list = std::exchange(other.cmd_list, nullptr);
			cmd_pool = std::exchange(other.cmd_pool, nullptr);

			return*this;
		}

		CommandNode(const CommandNode&) = delete;
		CommandNode& operator=(const CommandNode&) = delete;

		[[nodiscard]] auto acquire_cmd_list(bool& recording, std::string_view name) -> ICommandList* {
			EDGE_PROFILE_ZONE_SCOPED;

			if (!recording) {
				fence->wait();
				fence->reset();

				cmd_pool->reset();
				cmd_list = nullptr;

				cmd_list = cmd_pool->allocate();
				cmd_list->begin();
				cmd_list->begin_marker(name, 0xFFFF00FF);
				recording = true;
			}
			return cmd_list;
		}

		auto flush(bool& recording, SemaphoreSubmitInfo& signalled_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores) -> void {
			EDGE_PROFILE_ZONE_SCOPED;

			if (!recording)
				return;

			cmd_list->end_marker();
			cmd_list->end();

			signalled_semaphore.semaphore = semaphore;
			signalled_semaphore.value = ++counter;

			SubmitInfo submit_info{};
			submit_info.command_lists = { &cmd_list, 1 };
			submit_info.signal_semaphore_infos = { &signalled_semaphore, 1 };
			submit_info.wait_semaphore_infos = wait_semaphores;
			queue->submit(submit_info, fence);

			recording = false;
		}

		const IQueue* queue;

		IFence* fence;
		ISemaphore* semaphore;

		ICommandList* cmd_list{ nullptr };
		ICommandPool* cmd_pool;

		uint64_t counter{ 0 };
	};

	struct UploaderNode : CommandNode {
		UploaderNode(const IQueue* queue, uint64_t size, std::string_view name) :
			CommandNode{ queue, name } {
			allocator = std::make_unique<ArenaStagingAllocator>(size);
		}

		UploaderNode(UploaderNode&& other) noexcept :
			CommandNode{ std::move(other) },
			allocator{ std::move(other.allocator) } {
		}

		auto operator=(UploaderNode&& other) noexcept -> UploaderNode& {
			CommandNode::operator=(std::move(other));
			allocator = std::move(other.allocator);

			return*this;
		}

		UploaderNode(const UploaderNode&) = delete;
		UploaderNode& operator=(const UploaderNode&) = delete;

		std::unique_ptr<StagingBufferAllocator> allocator;
	};

	class CommandExecutor {
	public:
		CommandExecutor(const IQueue* queue, StageFlag stage, std::string_view name) :
			queue{ queue },
			stage{ stage },
			name{ name } {
		}

		auto flush(SemaphoreSubmitInfo& signalled_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores) -> void {
			EDGE_PROFILE_ZONE_SCOPED;

			auto& node = get_active_node();
			signalled_semaphore.stage = stage;
			node.flush(recording, signalled_semaphore, wait_semaphores);
			active_node_index = (active_node_index + 1) % nodes.size();
		}

		[[nodiscard]] virtual auto acquire_cmd_list() -> ICommandList* {
			EDGE_PROFILE_ZONE_SCOPED;

			auto& node = get_active_node();
			return node.acquire_cmd_list(recording, name);
		}

		auto get_active_node() -> CommandNode& {
			return *nodes[active_node_index];
		}
	protected:
		const IQueue* queue;
		std::vector<std::unique_ptr<CommandNode>> nodes;
		uint32_t active_node_index{ 0 };
		StageFlag stage;
		std::string_view name;
		bool recording{ false };
	};

	class MipGenerator : public CommandExecutor {
	public:
		MipGenerator(const IQueue* queue, uint32_t generator_count) :
			CommandExecutor{ queue, StageFlag::eComputeShader, "MIP_GENERATOR" } {
			const std::span<const uint8_t> mip_generate_shaders[] = {
				{ reinterpret_cast<const uint8_t*>(MipGeneratorShader), MipGeneratorShader_size },
				{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip2), MipGeneratorShaderMip2_size },
				{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip3), MipGeneratorShaderMip3_size },
				{ reinterpret_cast<const uint8_t*>(MipGeneratorShaderMip4), MipGeneratorShaderMip4_size }
			};

			// Load all shaders
			for (int32_t i = 0; i < shader_instances.size(); ++i) {
				auto& shader_instance = shader_instances[i];

				const edge::gfx::ShaderStageInfo shader_stages[] {
					{
						.stage = edge::gfx::ShaderStageFlag::eCompute,
						.code = mip_generate_shaders[i]
					}
				};

				shader_instance.shader = create_shader(shader_stages);

				RootSignatureCreateInfo root_sign_ci{};
				root_sign_ci.shader = shader_instance.shader;

				shader_instance.root_signature = create_root_signature(root_sign_ci);

				ComputePipelineCreateInfo pipeline_ci{};
				pipeline_ci.shader = shader_instance.shader;
				pipeline_ci.root_signature = shader_instance.root_signature;

				shader_instance.pipeline = create_pipeline(pipeline_ci);
			}

			// Create descriptor allocator for allocate descriptors in mip passes
			descriptor_allocator = create_descriptor_allocator();

			for (int32_t i = 0; i < generator_count; ++i) {
				auto debug_name = std::format("MIP_GENERAOTR_{}", i);
				nodes.emplace_back(std::make_unique<CommandNode>(queue, debug_name));
			}

			// Also we need linear sampler 
			SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = Filter::eLinear;
			sampler_ci.mag_filter = Filter::eLinear;
			sampler_ci.mip_map_mode = Filter::eLinear;

			linear_sampler = create_sampler(sampler_ci);
		}

		~MipGenerator() {
			// Delete mip generation resources
			for (int32_t i = 0; i < shader_instances.size(); ++i) {
				destroy(shader_instances[i].pipeline);
				destroy(shader_instances[i].root_signature);
				destroy(shader_instances[i].shader);
			}

			destroy(descriptor_allocator);
		}

		ResourceStateFlags write_commands(ITexture* texture, std::string_view path) {
			ICommandList* cmd_list = acquire_cmd_list();
			cmd_list->begin_marker(path, 0xFFFFFFFF);

			// ITexture should be in copy dest state
			ResourceStateFlags resource_state{ ResourceStateFlag::eCopyDst };

			TextureBarrierDesc texture_barrier{};
			texture_barrier.texture = texture;

			ResourceBarrierInfo resource_barrier{};
			resource_barrier.texture_barriers = { &texture_barrier, 1u };

			uint32_t total_mip_count = texture->get_mip_level_count();
			for (int32_t mip = 1; mip < total_mip_count;) {
				uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

				group_x = texture->get_width() >> mip;
				group_y = texture->get_height() >> mip;

				uint32_t mip_count{ 4u };
				if (mip + mip_count > total_mip_count)
					mip_count = total_mip_count - mip;

				auto& generator = shader_instances[mip_count - 1];

				IResourceDescriptor* descriptor = descriptor_allocator->allocate_descriptors(generator.root_signature);

				cmd_list->bind_pipeline(generator.pipeline);

				// At first make texture readable
				texture_barrier.src_state = resource_state;
				texture_barrier.dst_state = ResourceStateFlag::eRead | ResourceStateFlag::eCompute | ResourceStateFlag::eSample | ResourceStateFlag::eGeneral;
				cmd_list->resource_barrier(resource_barrier);
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
				descriptor->bind_texture(bind_resource);

				// Make texture writable now
				texture_barrier.src_state = resource_state;
				texture_barrier.dst_state = ResourceStateFlag::eWrite | ResourceStateFlag::eCompute | ResourceStateFlag::eStorage | ResourceStateFlag::eGeneral;
				cmd_list->resource_barrier(resource_barrier);
				resource_state = texture_barrier.dst_state;

				bind_resource.type = DescriptorType::eStorageImage;
				bind_resource.resource_state = resource_state;

				for (int32_t i = 0; i < mip_count; ++i) {
					bind_resource.binding = i + 1;
					bind_resource.subresource_index = mip + i;
					descriptor->bind_texture(bind_resource);
				}

				// Finally bind descriptor
				BindDescriptorsInfo bind_info{};
				bind_info.pipeline_type = PipelineType::eCompute;
				bind_info.resource_descriptors = { &descriptor, 1u };
				bind_info.root_signature = generator.root_signature;

				descriptor->flush();
				cmd_list->bind_descriptors(bind_info);

				// Divide by local group sizes
				auto& reflection = generator.shader->get_reflection();
				group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
				group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
				group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);

				cmd_list->dispatch(group_x, group_y, group_z);

				mip += mip_count;
			}

			cmd_list->end_marker();

			return resource_state;
		}

		void reset() {
			descriptor_allocator->reset();
		}
	private:
		struct ShaderInstance {
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
		src_offset{ src_offset } {
		src_memory = src_buffer->map();
	}

	void BufferUpdater::add_region(std::span<const uint8_t> data) {
		assert(data.size() <= src_region_size && (dst_offset + data.size() <= copy_buffer_info.dst_buffer->get_size()) && "Trying to add region out of range.");

		copy_regions.push_back({
				src_offset,
				dst_offset,
				data.size()
			});

		memcpy(src_memory + src_offset, data.data(), data.size());

		src_offset += data.size();
		dst_offset += data.size();
	}

	void BufferUpdater::insert_region(std::span<const uint8_t> data, uint64_t offset) {
		assert(data.size() <= src_region_size && (offset + data.size() <= copy_buffer_info.dst_buffer->get_size()) && "Trying to insert region out of range.");

		copy_regions.push_back({
				src_offset,
				offset,
				data.size()
			});

		//engine::util::Copy(src_memory, src_offset, data.data(), data.size(), offset);

		memcpy(src_memory + src_offset, data.data(), data.size());

		src_offset += data.size();
	}

	void BufferUpdater::submit(ICommandList* cmd_list, ResourceStateFlags& current_state) {
		copy_buffer_info.src_buffer->unmap();

		// Sort regions by destination offset
		std::sort(copy_regions.begin(), copy_regions.end(),
			[](const BufferCopyRegion& a, const BufferCopyRegion& b) {
				return a.dst_offset < b.dst_offset;
			});

		std::vector<BufferCopyRegion> merged_regions{};
		merged_regions.emplace_back(copy_regions.front());

		for (int32_t i = 1; i < copy_regions.size(); ++i) {
			BufferCopyRegion& last = merged_regions.back();
			const BufferCopyRegion& current = copy_regions[i];

			uint64_t last_end = last.dst_offset + last.size;
			uint64_t current_end = current.dst_offset + current.size;

			// Check if regions are contiguous or overlapping
			if (last_end >= current.dst_offset &&
				(last.src_offset + (current.dst_offset - last.dst_offset) == current.src_offset)) {

				// Merge regions by expanding the last region
				last.size = std::max(last_end, current_end) - last.dst_offset;
			}
			else {
				// Regions can't be merged; add a new region
				merged_regions.push_back(current);
			}
		}

		if (current_state != ResourceStateFlag::eCopyDst) {
			// Translate dst_buffer for correct layout
			BufferBarrierDesc buffer_barrier{};
			buffer_barrier.buffer = copy_buffer_info.dst_buffer;
			buffer_barrier.src_state = current_state;
			current_state = buffer_barrier.dst_state = ResourceStateFlag::eCopyDst;
			buffer_barrier.range.size = src_region_size;

			ResourceBarrierInfo resource_barrier{};
			resource_barrier.buffer_barriers = { &buffer_barrier, 1 };

			cmd_list->resource_barrier(resource_barrier);
		}

		copy_buffer_info.regions = merged_regions;
		cmd_list->copy_buffer(copy_buffer_info);
	}

	class ResourceLoader final : public CommandExecutor {
	public:
		ResourceLoader(const IQueue* queue, uint32_t node_count, uint64_t buffer_size, std::string_view name = {}) :
			CommandExecutor{ queue, StageFlag::eTransfer, name } {
			for (int32_t i = 0; i < node_count; ++i) {
				auto debug_name = std::format("{}_{}", name, i);
				nodes.emplace_back(std::make_unique<UploaderNode>(queue, buffer_size, debug_name));
			}
		}

		[[nodiscard]] auto acquire_cmd_list() -> ICommandList* override {
			EDGE_PROFILE_ZONE_SCOPED;

			auto& node = (UploaderNode&)get_active_node();
			if (!recording) {
				node.allocator->reset();
			}

			return node.acquire_cmd_list(recording, name);
		}
	};

	std::unique_ptr<CommandExecutor> ResourceUpdater::updater{};
	std::unique_ptr<UploadingDispatcher> ResourceUpdater::uploading_dispatcher{};

	void ResourceUpdater::create(const ResourceUpdaterCreateInfo& create_info) {

		auto update_queue = request_queue(QueueTypeFlag::eGraphics);
		update_queue->set_resource_name("UPDATE");
		updater = std::make_unique<ResourceLoader>(update_queue, create_info.update_buffer_count, create_info.update_buffer_size, "UPDATER");

		//auto compute_queue = request_queue(QueueTypeFlag::eCompute, QueueTypeFlag::eGraphics);
		//compute_queue->set_resource_name("COMPUTE_ASYNC");
		//mip_generator = std::make_unique<MipGenerator>(compute_queue, create_info.buffer_count);

		uploading_dispatcher = std::make_unique<UploadingDispatcher>(create_info.upload_buffer_size, create_info.upload_buffer_count);
	}

	void ResourceUpdater::shutdown() {
		updater = nullptr;
		uploading_dispatcher = nullptr;
		//mip_generator = nullptr;
	}

	auto ResourceUpdater::load_texture(std::string path) -> std::shared_ptr<AsyncResourceHandle> {
		return uploading_dispatcher->submit(
			[path](ICommandList* cmd_list, StagingBufferAllocator& allocator) -> ResourceUploadResult {
				EDGE_PROFILE_ZONE_SCOPED;

				ResourceStateFlags state{};
				return std::make_pair(load_texture_impl(cmd_list, allocator, path, state), state);
			});
	}

	auto ResourceUpdater::load_texture_sync(std::string path, ResourceStateFlags& state) -> ITexture* {
		EDGE_PROFILE_ZONE_SCOPED;

		ICommandList* cmd_list = updater->acquire_cmd_list();
		auto& node = (UploaderNode&)updater->get_active_node();
		return load_texture_impl(cmd_list, *node.allocator, path, state);
	}

	auto ResourceUpdater::sync_uploads() -> void {
		//needs_sync.store(true, std::memory_order_relaxed);
	}

	void ResourceUpdater::update_texture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data) {
		EDGE_PROFILE_ZONE_SCOPED;

		ICommandList* cmd_list = updater->acquire_cmd_list();

		auto& node = (UploaderNode&)updater->get_active_node();
		auto [src_buffer, src_offset] = node.allocator->allocate(data.size());

		auto* src_memory = src_buffer->map();
		memcpy(src_memory + src_offset, data.data(), data.size());

		TextureCopyRegion copy_region{};
		copy_region.buffer_offset = src_offset;
		copy_region.size_x = dst_texture->get_width();
		copy_region.size_y = dst_texture->get_height();
		copy_region.size_z = dst_texture->get_depth();

		CopyTextureInfo copy_texture{};
		copy_texture.dst_texture = dst_texture;
		copy_texture.src_buffer = src_buffer;
		copy_texture.copy_regions = { &copy_region, 1 };

		TextureBarrierDesc texture_barrier{};
		texture_barrier.texture = dst_texture;
		texture_barrier.src_state = current_state;
		current_state = texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

		ResourceBarrierInfo resource_barrier{};
		resource_barrier.texture_barriers = { &texture_barrier, 1 };

		// Transfer to intermediate layout
		cmd_list->resource_barrier(resource_barrier);

		// Copy texture regions
		cmd_list->copy_texture(copy_texture);

		// Transfer to final layout
		texture_barrier.src_state = current_state;
		current_state = texture_barrier.dst_state = ResourceStateFlag::eShaderReadOnlyResource;
		cmd_list->resource_barrier(resource_barrier);
	}

	bool ResourceUpdater::flush(std::vector<SemaphoreSubmitInfo>& signalled_semaphores, std::span<SemaphoreSubmitInfo> wait_semaphores) {
		SemaphoreSubmitInfo updater_semaphore{};
		updater->flush(updater_semaphore, wait_semaphores);
		if (updater_semaphore.semaphore)
			signalled_semaphores.emplace_back(updater_semaphore);

		bool uploads_flushed{ false };
		SemaphoreSubmitInfo uploader_semaphore{};
		if (uploading_dispatcher->is_sync_needed(uploader_semaphore)) {
			signalled_semaphores.emplace_back(uploader_semaphore);
			uploads_flushed = true;
		}

		return uploads_flushed;
	}

	auto ResourceUpdater::get_buffer_updater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater {
		updater->acquire_cmd_list();

		auto& node = (UploaderNode&)updater->get_active_node();
		auto [src_buffer, src_offset] = node.allocator->allocate(update_size);

		return { src_buffer, src_offset, update_size, dst_buffer };
	}

	void ResourceUpdater::submit_buffer_updater(BufferUpdater& buffer_updater, ResourceStateFlags& current_state) {
		ICommandList* cmd_list = updater->acquire_cmd_list();
		buffer_updater.submit(cmd_list, current_state);
	}

	void ResourceUpdater::update_buffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, 
		std::span<const uint8_t> data) {
		auto updater = get_buffer_updater(dst_buffer, data.size());
		updater.add_region(data);
		submit_buffer_updater(updater, current_state);
	}

	auto ResourceUpdater::load_texture_impl(ICommandList* cmd_list, StagingBufferAllocator& allocator, std::string path, 
		ResourceStateFlags& output_state) -> ITexture* {
		EDGE_LOGT("[ResourceUpdater] Loading texture \"{}\"", path);
		cmd_list->begin_marker(path, 0xFFFFFFFF);

		auto texture_loader = make_texture_loader(path);

		ITexture* dst_texture = texture_loader->get_texture();
		dst_texture->set_resource_name(path);

		TextureBarrierDesc texture_barrier{};
		texture_barrier.texture = dst_texture;
		texture_barrier.src_state = ResourceStateFlag::eUndefined;
		texture_barrier.dst_state = ResourceStateFlag::eCopyDst;

		ResourceBarrierInfo resource_barrier{};
		resource_barrier.texture_barriers = { &texture_barrier, 1 };

		// Transfer to intermediate layout
		cmd_list->resource_barrier(resource_barrier);

		texture_loader->upload(cmd_list, allocator);

		cmd_list->end_marker();

		output_state = ResourceStateFlag::eCopyDst;

		EDGE_LOGT("[ResourceUpdater] Loading texture \"{}\" done", path);

		//if (texture_reader->IsNeedToGenerateMips())
		//	resource_result.state = mip_generator->WriteCommands(dst_texture, path);

		return dst_texture;
	}
}
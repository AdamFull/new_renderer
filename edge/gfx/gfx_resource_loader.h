#pragma once

#include <queue>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "gfx_structures.h"

namespace edge::gfx {
	class StagingBufferAllocator;
	class UploadingDispatcher;
	class CommandExecutor;
	class MipGenerator;

	class BufferUpdater {
	public:
		friend class ResourceUpdater;

		BufferUpdater(IBuffer* src_buffer, uint64_t src_offset, uint64_t src_size, IBuffer* dst_buffer);

		void add_region(std::span<const uint8_t> data);
		void insert_region(std::span<const uint8_t> data, uint64_t dst_offset);
		void submit(ICommandList* command_list, ResourceStateFlags& current_state);

	private:
		CopyBufferInfo copy_buffer_info{};
		uint64_t src_region_size;
		uint64_t src_offset{ 0ull };
		uint64_t dst_offset{ 0ull };
		uint8_t* src_memory;

		std::vector<BufferCopyRegion> copy_regions{};
	};

	class TextureUpdater {
	public:
		friend class ResourceUpdater;

		TextureUpdater(IBuffer* src_buffer, uint64_t src_size, uint64_t src_offset, ITexture* dst_texture);
	private:
		void submit(ICommandList* command_list);

		CopyTextureInfo copy_texture_info{};
		uint64_t src_region_size;
		uint64_t src_offset{ 0ull };

		std::vector<TextureCopyRegion> copy_regions{};
	};

	struct ResourceUpdaterCreateInfo {
		uint64_t update_buffer_size{ 16ull * 1024ull * 1024ull };
		uint32_t update_buffer_count{ 1u };

		uint64_t upload_buffer_size{ 32ull * 1024ull * 1024ull };
		uint32_t upload_buffer_count{ 2u };
	};

	void make_resource_uploader(const ResourceUpdaterCreateInfo& create_info);
	void shutdown_resource_uploader();

	bool flush_resource_uploads(std::vector<SemaphoreSubmitInfo>& signalled_semaphores, std::span<SemaphoreSubmitInfo> wait_semaphores);

	auto load_texture(std::string path) -> std::shared_ptr<GFXResourcePromise>;
	auto load_texture_sync(std::string path, ResourceStateFlags& state) -> ITexture*;
	auto sync_uploads() -> void;

	void update_texture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data);

	auto get_buffer_updater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater;
	void submit_buffer_updater(BufferUpdater& updater, ResourceStateFlags& current_state);
	void update_buffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, std::span<const uint8_t> data);
}
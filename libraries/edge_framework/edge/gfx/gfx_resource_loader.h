#pragma once

#include <queue>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "../util/utl_future.hpp"

#include "gfx_common.h"
#include "gfx_structures.h"

namespace edge
{
	namespace fs
	{
		class VirtualFilesystem;
	}

	namespace gfx
	{
		class StagingBufferAllocator;
		class UploadingDispatcher;
		class CommandExecutor;
		class MipGenerator;

		class BufferUpdater
		{
		public:
			friend class ResourceUpdater;

			BufferUpdater(IBuffer* src_buffer, uint64_t src_offset, uint64_t src_size, IBuffer* dst_buffer);

			void AddRegion(std::span<const uint8_t> data);
			void InsertRegion(std::span<const uint8_t> data, uint64_t dst_offset);
		private:
			void Submit(ICommandList* command_list, ResourceStateFlags& current_state);

			CopyBufferInfo copy_buffer_info{};
			uint64_t src_region_size;
			uint64_t src_offset{ 0ull };
			uint64_t dst_offset{ 0ull };
			uint8_t* src_memory;

			std::vector<BufferCopyRegion> copy_regions{};
		};

		class TextureUpdater
		{
		public:
			friend class ResourceUpdater;

			TextureUpdater(IBuffer* src_buffer, uint64_t src_size, uint64_t src_offset, ITexture* dst_texture);
		private:
			void Submit(ICommandList* command_list);

			CopyTextureInfo copy_texture_info{};
			uint64_t src_region_size;
			uint64_t src_offset{ 0ull };

			std::vector<TextureCopyRegion> copy_regions{};
		};

		struct ResourceUpdaterCreateInfo
		{
			IContext* gfx_context{ nullptr };
			fs::VirtualFilesystem* filesystem{ nullptr };

			uint64_t buffer_size{ 32ull * 1024ull * 1024ull };
			uint32_t buffer_count{ 1u };
		};

		class ResourceUpdater
		{
		public:
			static void Create(const ResourceUpdaterCreateInfo& create_info);
			static void Shutdown();

			static bool Flush(std::vector<SemaphoreSubmitInfo>& signalled_semaphores, std::span<SemaphoreSubmitInfo> wait_semaphores);

			static auto LoadTexture(std::string path) -> std::shared_ptr<AsyncResourceHandle>;
			static auto LoadTextureSync(std::string path, ResourceStateFlags& state) -> ITexture*;
			static auto SyncUploads() -> void;

			static void UpdateTexture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data);

			static auto GetBufferUpdater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater;
			static void SubmitBufferUpdater(BufferUpdater& updater, ResourceStateFlags& current_state);
			static void UpdateBuffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, std::span<const uint8_t> data);

			//static auto GetTextureUpdater(ITexture* dst_texture, uint64_t update_size) -> TextureUpdater;
		private:
			static auto LoadTextureImpl(ICommandList* cmd_list, StagingBufferAllocator& allocator, std::string path, ResourceStateFlags& output_state) -> ITexture*;

			static IContext* gfx_context;
			static fs::VirtualFilesystem* filesystem;

			static std::unique_ptr<CommandExecutor> updater;
			//static std::unique_ptr<CommandExecutor> uploader;
			//static std::unique_ptr<MipGenerator> mip_generator;
			static std::unique_ptr<UploadingDispatcher> uploading_dispatcher;
		};
	}
}
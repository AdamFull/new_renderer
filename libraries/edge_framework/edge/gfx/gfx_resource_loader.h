#pragma once

#include <thread>
#include <memory>

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

		struct ResourceNode;

		class IResourceLoader
		{
		public:
			virtual ~IResourceLoader() = default;

			[[nodiscard]] virtual auto AcquireCommandList() -> ICommandList* = 0;
			virtual auto Flush(SemaphoreSubmitInfo& submitted_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores) -> void = 0;
			virtual auto GetActiveNode() -> ResourceNode& = 0;
		};

		class ResourceUpdater
		{
		public:
			static void Create(const ResourceUpdaterCreateInfo& create_info);
			static void Shutdown();

			static void Flush(SemaphoreSubmitInfo& submitted_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores);

			static auto LoadTexture(std::string_view path) -> ITexture*;

			static void UpdateTexture(ITexture* dst_texture, ResourceStateFlags& current_state, std::span<const uint8_t> data);

			static auto GetBufferUpdater(IBuffer* dst_buffer, uint64_t update_size) -> BufferUpdater;
			static void SubmitBufferUpdater(BufferUpdater& updater, ResourceStateFlags& current_state);
			static void UpdateBuffer(IBuffer* dst_buffer, ResourceStateFlags& current_state, std::span<const uint8_t> data);

			//static auto GetTextureUpdater(ITexture* dst_texture, uint64_t update_size) -> TextureUpdater;
		private:
			static IContext* gfx_context;
			static fs::VirtualFilesystem* filesystem;
			static const IQueue* queue;

			static std::unique_ptr<IResourceLoader> loader;
			static std::unique_ptr<MipGenerator> mip_generator;
		};

		class ResourceUploader
		{
		public:
			static void Create(const ResourceUpdaterCreateInfo& create_info);
			static void Shutdown();

			static void Flush(SemaphoreSubmitInfo& submitted_semaphore, std::span<SemaphoreSubmitInfo> wait_semaphores);

			static auto LoadTexture(std::string_view path, ResourceStateFlags& output_resource_state) -> ITexture*;

			static void UploaderThread(std::stop_token stop_token);
		private:
			static IContext* gfx_context;
			static fs::VirtualFilesystem* filesystem;
			static const IQueue* queue;

			static std::unique_ptr<IResourceLoader> loader;
			static std::unique_ptr<MipGenerator> mip_generator;

			static std::jthread upload_thread;
			static std::stop_source uploader_stop;
		};
	}
}
#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/UploadFrame.h>

namespace engine
{
	namespace render
	{
		struct BufferUpdateInfo2
		{
			gfx::Buffer* dst_buffer{ nullptr };
			uint64_t update_size{ 0ull };

			struct
			{
				uint8_t* data{ nullptr };
				uint64_t avaliable_size{ 0ull };
				size_t buffer_index{ ~0ull };
			} map_info;
		};

		class UploadContext final
		{
		public:
			UploadContext(gfx::Context& context, filesystem::IVirtualFileSystemInterface* vfs);
			~UploadContext() = default;

			UploadContext(const UploadContext&) = delete;
			UploadContext(UploadContext&&) = delete;
			UploadContext& operator=(const UploadContext&) = delete;
			UploadContext& operator=(UploadContext&&) = delete;

			gfx::Texture* LoadTexture(const std::string& texture_path);

			//void UpdateTexture(gfx::Texture* texture_handle, const FTextureUploadInfo& upload_info);

			//void BeginTextureUpdate(VkTextureUpdateInfo& update_info);
			//void EndTextureUpdate(VkTextureUpdateInfo& update_info);

			void BeginBufferUpdate(BufferUpdateInfo2& update_info);
			void EndBufferUpdate(BufferUpdateInfo2& update_info);

			gfx::SemaphoreSubmitInfo Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count);
		private:
			UploadFrame& GetActiveFrame();
			void BeginFrame();
		private:
			gfx::Context& context;
			filesystem::IVirtualFileSystemInterface* vfs{};

			const gfx::Queue* queue{ nullptr };

			static constexpr size_t k_max_upload_frames{ 8ull };
			std::array<std::unique_ptr<UploadFrame>, k_max_upload_frames> upload_frames{};
			uint32_t upload_frame_count{ 1u };
			uint32_t active_upload_frame{ 0u };

			bool writing_commands{ false };
		};
	}
}
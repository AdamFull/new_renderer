#pragma once

#include <engine/render/render_api/vulkan/UploadFrame.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Queue;
			class RenderDevice;
			class CommandPool;
			class CommandBuffer;
			class Buffer;
			class Texture;

			class UploadContext final
			{
			public:
				UploadContext(RenderDevice& render_device);
				~UploadContext();

				UploadContext(const UploadContext&) = delete;
				UploadContext(UploadContext&&) = delete;
				UploadContext& operator=(const UploadContext&) = delete;
				UploadContext& operator=(UploadContext&&) = delete;

				void UpdateTexture(Texture* texture_handle, const FTextureUploadInfo& upload_info);

				void BeginTextureUpdate(Texture* texture_handle);
				void EndTextureUpdate();

				void BeginBufferUpdate(VkBufferUpdateInfo& update_info);
				void EndBufferUpdate(VkBufferUpdateInfo& update_info);

				vk::SemaphoreSubmitInfo Submit(vk::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count);
			private:
				UploadFrame& GetActiveFrame();
				void BeginFrame();
			private:
				RenderDevice& render_device;

				const Queue& queue;

				std::vector<std::unique_ptr<UploadFrame>> upload_frames{};
				uint32_t active_upload_frame{ 0u };

				bool writing_commands{ false };
			};
		}
	}
}
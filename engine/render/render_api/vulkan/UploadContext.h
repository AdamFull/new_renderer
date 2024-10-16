#pragma once

#include <engine/render/render_api/vulkan/VkResourceStructures.h>

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
				void UploadBuffer(Buffer* buffer_handle, const FBufferUploadInfo& upload_info);

				vk::Semaphore Submit();
			private:
				void BeginCommands();
			private:
				RenderDevice& render_device;

				const Queue& queue;

				vk::Semaphore signal_semaphore{};

				std::unique_ptr<CommandPool> command_pool;
				CommandBuffer* active_command_buffer{ nullptr };

				utl::vector<VkTextureUploadInfo> texture_upload_infos{};
				utl::vector<VkBufferUploadInfo> buffer_upload_infos{};

				utl::vector<std::unique_ptr<Buffer>> upload_buffers;

				bool writing_commands{ false };
			};
		}
	}
}
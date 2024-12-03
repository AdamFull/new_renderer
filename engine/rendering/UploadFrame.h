#pragma once

#include <gfx/gfx_structures.h>

namespace engine
{
	namespace gfx
	{
		class Context;
		class Queue;
		class Fence;
		class Semaphore;
		class Buffer;
		class CommandList;
		class CommandPool;
	}

	namespace render
	{
		class UploadFrame final
		{
		public:
			UploadFrame(gfx::Context& context, const gfx::Queue& queue);
			~UploadFrame();

			UploadFrame(const UploadFrame&) = delete;
			UploadFrame(UploadFrame&&) = delete;
			UploadFrame& operator=(const UploadFrame&) = delete;
			UploadFrame& operator=(UploadFrame&&) = delete;

			gfx::Buffer& RequestStagingBuffer(size_t size, size_t* buffer_index = nullptr);
			gfx::Buffer& GetStagingBuffer(size_t buffer_index);

			gfx::CommandList& AcquireOrGetCommandList();

			gfx::SemaphoreSubmitInfo Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t semaphore_count);

			void Reset();
		private:
			gfx::Context& context;

			const gfx::Queue& queue;

			gfx::CommandPool* command_pool{ nullptr };
			gfx::CommandList* active_command_list{ nullptr };

			std::vector<gfx::Buffer*> staging_buffers;

			gfx::Semaphore* signal_semaphore{ nullptr };
			gfx::Fence* frame_fence{ nullptr };
		};
	}
}
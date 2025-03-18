#pragma once

#include <queue>
#include <thread>
#include <future>
#include <mutex>

#include "../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class StagingBufferAllocator;

		using ResourceUploadResult = std::pair<IGFXResource*, ResourceStateFlags>;
		using ResourceUploadFnSign = ResourceUploadResult(ICommandList*, StagingBufferAllocator&);

		struct ResourceJobPack
		{
			std::shared_ptr<AsyncResourceHandle> handle{};
			std::packaged_task<ResourceUploadFnSign> task{};
		};

		class UploadingThreadWorker
		{
		public:
			UploadingThreadWorker(IContext* gfx_context, const IQueue* queue, uint64_t buffer_size, const std::string& worker_name);
			~UploadingThreadWorker();

			UploadingThreadWorker(UploadingThreadWorker&& other) noexcept = delete;
			auto operator=(UploadingThreadWorker&& other) noexcept -> UploadingThreadWorker & = delete;
			UploadingThreadWorker(const UploadingThreadWorker&) = delete;
			UploadingThreadWorker& operator=(const UploadingThreadWorker&) = delete;

			auto DispatchJob(ResourceJobPack&& job) -> void;
			auto EndCmd() noexcept -> ICommandList*;
			auto GetWorkCount() noexcept -> uint64_t;
		private:
			auto AcquireCmdList() -> ICommandList*;
			auto Loop(std::stop_token stop_token) -> void;

			IContext* gfx_context;
			ICommandPool* cmd_pool;
			ICommandList* cmd_list{ nullptr };

			std::string worker_name;

			std::jthread thread;
			std::mutex mutex;
			std::condition_variable cv;
			std::queue<ResourceJobPack> jobs{};
			std::atomic_uint64_t work_count{};

			bool recording{ false };

			std::unique_ptr<StagingBufferAllocator> allocator;
		};

		class UploadingDispatcher
		{
		public:
			UploadingDispatcher(IContext* gfx_context, uint64_t buffer_size = 16 * 1024 * 1024, uint32_t worker_count = 2);
			~UploadingDispatcher();

			auto Submit(ResourceJobPack&& job) -> void;
			auto IsSyncNeeded(SemaphoreSubmitInfo& semaphore_submit) -> bool;
		private:
			auto Loop(std::stop_token stop_token) -> void;

			IContext* gfx_context;
			const IQueue* queue;

			IFence* fence;
			SemaphoreSubmitInfo signal_semaphore{};
			std::atomic_bool sync_needed{};

			std::jthread dispatcher_thread;
			std::mutex mutex;
			std::mutex semaphore_mutex;
			std::condition_variable cv;

			std::vector<std::shared_ptr<AsyncResourceHandle>> processed_handles{};
			std::queue<ResourceJobPack> pending_jobs{};
			std::vector<std::unique_ptr<UploadingThreadWorker>> workers{};
			size_t worker_index{ 0 };
		};
	}
}
#pragma once

#include <queue>
#include <thread>
#include <future>
#include <mutex>

#include "../gfx_structures.h"
#include "../../util/utl_thread_pool.h"

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

		struct WorkerContext
		{
		public:
			WorkerContext(IContext* gfx_context, const IQueue* queue, uint64_t buffer_size);
			~WorkerContext();

			WorkerContext(WorkerContext&& other) noexcept = delete;
			auto operator=(WorkerContext&& other) noexcept -> WorkerContext & = delete;
			WorkerContext(const WorkerContext&) = delete;
			WorkerContext& operator=(const WorkerContext&) = delete;

			auto AcquireCmdList() -> ICommandList*;
			auto EndCmd() noexcept -> ICommandList*;

			IContext* gfx_context;
			ICommandPool* cmd_pool;
			ICommandList* cmd_list{ nullptr };

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
			auto GetWorkerContext() -> WorkerContext&;
		private:
			auto Loop(std::stop_token stop_token) -> void;
			auto RegisterThreadContext() -> size_t;

			IContext* gfx_context;
			const IQueue* queue;

			uint64_t buffer_size;

			utl::ThreadPool pool;

			IFence* fence;
			SemaphoreSubmitInfo signal_semaphore{};
			std::atomic_bool sync_needed{};

			std::jthread dispatcher_thread;
			std::mutex mutex;
			std::mutex semaphore_mutex;
			std::condition_variable cv;

			std::vector<std::unique_ptr<WorkerContext>> contexts;
			std::mutex context_mtx;
			size_t next_worker{ 0ull };

			std::vector<std::shared_ptr<AsyncResourceHandle>> processed_handles{};
			std::queue<ResourceJobPack> pending_jobs{};
		};
	}
}
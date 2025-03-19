#pragma once

#include <queue>
#include <thread>
#include <future>
#include <mutex>

#include "../gfx_structures.h"

#include <taskflow/taskflow.hpp>

namespace edge
{
	namespace gfx
	{
		class StagingBufferAllocator;

		using ResourceUploadResult = std::pair<IGFXResource*, ResourceStateFlags>;
		using ResourceUploadFnSign = ResourceUploadResult(ICommandList*, StagingBufferAllocator&);

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

		struct UploadContext
		{
			UploadContext(IContext* gfx_context);
			~UploadContext();

			void Reset();
			void IncrementSemaphore();
			void AddWorker(const IQueue* queue, uint64_t size);

			IContext* gfx_context;
			IFence* fence;
			SemaphoreSubmitInfo signal_semaphore{};

			std::vector<std::unique_ptr<WorkerContext>> workers;
		private:
			UploadContext(UploadContext&& other) noexcept = delete;
			auto operator=(UploadContext&& other) noexcept -> UploadContext & = delete;
			UploadContext(const UploadContext&) = delete;
			UploadContext& operator=(const UploadContext&) = delete;
		};

		class UploadingDispatcher
		{
		public:
			UploadingDispatcher(IContext* gfx_context, uint64_t buffer_size = 16 * 1024 * 1024, uint32_t uploader_count = 2);
			~UploadingDispatcher();

			template<typename _Fn>
			auto Submit(_Fn&& task) -> std::shared_ptr<AsyncResourceHandle>
			{
				auto handle = std::make_shared<AsyncResourceHandle>();

				{
					std::lock_guard lock(mutex);
					pending_jobs.emplace([this, handle, 
						t = std::make_shared<std::packaged_task<ResourceUploadFnSign>>(std::move(task))]() {
							auto& worker_ctx = GetWorkerContext();
							ICommandList* cmd = worker_ctx.AcquireCmdList();
							(*t)(cmd, *worker_ctx.allocator);

							auto result = t->get_future().get();
							handle->resource = result.first;
							handle->state = result.second;
						});
					pending_handles.emplace_back(handle);
				}
				cv.notify_one();

				return handle;
			}

			auto IsSyncNeeded(SemaphoreSubmitInfo& semaphore_submit) -> bool;
			auto GetWorkerContext() -> WorkerContext&;
		private:
			auto GetUploadContext() -> UploadContext&;
			auto Loop(std::stop_token stop_token) -> void;
			auto RegisterThreadContext() -> size_t;

			IContext* gfx_context;
			const IQueue* queue;

			uint64_t buffer_size;

			tf::Executor executor;
			tf::Taskflow pending_jobs;

			std::atomic_bool sync_needed{};

			std::jthread dispatcher_thread;
			std::mutex mutex;
			std::mutex semaphore_mutex;
			std::condition_variable cv;

			std::vector<std::unique_ptr<UploadContext>> upload_contexts{};
			uint32_t active_uploader_idx{ 0 };
			uint32_t last_used_uploader_idx{ 0 };
			size_t next_worker{ 0ull };
			std::mutex context_mtx;

			std::vector<std::shared_ptr<AsyncResourceHandle>> pending_handles{};
		};
	}
}
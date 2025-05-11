#pragma once

#include <atomic>
#include <queue>
#include <thread>
#include <future>
#include <mutex>

#include "../gfx_structures.h"

#include <taskflow/taskflow.hpp>

namespace edge::gfx
{
	class StagingBufferAllocator;

	using ResourceUploadResult = std::pair<IGFXResource*, ResourceStateFlags>;
	using ResourceUploadFnSign = ResourceUploadResult(ICommandList*, StagingBufferAllocator&);

	struct WorkerContext {
	public:
		WorkerContext(const IQueue* queue, uint64_t buffer_size);
		~WorkerContext();

		WorkerContext(WorkerContext&& other) noexcept = delete;
		auto operator=(WorkerContext&& other) noexcept -> WorkerContext & = delete;
		WorkerContext(const WorkerContext&) = delete;
		WorkerContext& operator=(const WorkerContext&) = delete;

		auto acquire_cmd_list() -> ICommandList*;
		auto end_cmd() noexcept -> ICommandList*;

		ICommandPool* cmd_pool;
		ICommandList* cmd_list{ nullptr };

		bool recording{ false };
		std::unique_ptr<StagingBufferAllocator> allocator;
	};

	struct UploadContext
	{
		UploadContext();
		~UploadContext();

		void reset();
		void increment_semaphore();
		void add_worker(const IQueue* queue, uint64_t size);

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
		UploadingDispatcher(uint64_t buffer_size = 16 * 1024 * 1024, uint32_t uploader_count = 2);
		~UploadingDispatcher();

		template<typename _Fn>
		auto submit(_Fn&& task) -> std::shared_ptr<GFXResourcePromise>
		{
			auto handle = std::make_shared<GFXResourcePromise>();

			{
				std::lock_guard lock(mutex);
				pending_jobs.emplace([this, handle,
					t = std::make_shared<std::packaged_task<ResourceUploadFnSign>>(std::move(task))]() {
						auto& worker_ctx = get_worker_context();
						ICommandList* cmd = worker_ctx.acquire_cmd_list();
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

		auto is_sync_needed(SemaphoreSubmitInfo& semaphore_submit) -> bool;
		auto get_worker_context() -> WorkerContext&;
	private:
		auto get_upload_context() -> UploadContext&;
		auto loop() -> void;
		auto register_thread_context() -> size_t;

		const IQueue* queue;

		uint64_t buffer_size;

		tf::Executor executor;
		tf::Taskflow pending_jobs;

		std::atomic_bool sync_needed{};

		std::thread dispatcher_thread;
        std::atomic_bool should_stop{false};
		std::mutex mutex;
		std::mutex semaphore_mutex;
		std::condition_variable cv;

		std::vector<std::unique_ptr<UploadContext>> upload_contexts{};
		uint32_t active_uploader_idx{ 0 };
		uint32_t last_used_uploader_idx{ 0 };
		size_t next_worker{ 0ull };
		std::mutex context_mtx;

		std::vector<std::shared_ptr<GFXResourcePromise>> pending_handles{};
	};
}
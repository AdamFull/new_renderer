#include "gfx_resource_uploader.h"

#include "../../util/Logger.h"
#include "../../util/utl_profiler.h"

#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

#include <chrono>

namespace edge::gfx
{
	WorkerContext::WorkerContext(const IQueue* queue, uint64_t buffer_size) {
		allocator = std::make_unique<ArenaStagingAllocator>(buffer_size);
		cmd_pool = create_command_pool(queue);
	}

	WorkerContext::~WorkerContext() {
		destroy(cmd_pool);
	}

	auto WorkerContext::acquire_cmd_list() -> ICommandList* {
		if (!recording) {
			allocator->reset();

			cmd_pool->reset();

			cmd_list = cmd_pool->allocate();
			cmd_list->begin();
			//cmd_list->begin_marker(worker_name, 0xFFFFFFFF);
			recording = true;
		}

		return cmd_list;
	}

	auto WorkerContext::end_cmd() noexcept -> ICommandList* {
		ICommandList* out_cmd_list{ nullptr };
		if (recording) {
			//cmd_list->end_marker();
			cmd_list->end();
			out_cmd_list = cmd_list;
			recording = false;
		}

		return out_cmd_list;
	}



	UploadContext::UploadContext() {
		fence = create_fence();
		signal_semaphore.semaphore = create_semaphore(SemaphoreType::eTimeline);
		signal_semaphore.value = 0;
		signal_semaphore.stage = StageFlag::eTransfer;
	}

	UploadContext::~UploadContext() {
		fence->wait();
		destroy(fence);
		destroy(signal_semaphore.semaphore);
	}

	void UploadContext::reset() {
		fence->wait();
		fence->reset();
	}

	void UploadContext::increment_semaphore() {
		++signal_semaphore.value;
	}

	void UploadContext::add_worker(const IQueue* queue, uint64_t size) {
		workers.emplace_back(std::make_unique<WorkerContext>(queue, size));
	}


	UploadingDispatcher::UploadingDispatcher(uint64_t buffer_size, uint32_t uploader_count) :
		buffer_size{ buffer_size },
		executor{ 2 } {

		queue = get_queue(QueueType::eCopy);
		queue->set_resource_name("UPLOAD");

		for (int32_t i = 0; i < uploader_count; ++i)
			upload_contexts.emplace_back(std::make_unique<UploadContext>());
		
		dispatcher_thread = std::thread([this]() { loop(); });
	}

	UploadingDispatcher::~UploadingDispatcher() {
        should_stop.store(true, std::memory_order_relaxed);
		cv.notify_all();

		upload_contexts.clear();
	}

	auto UploadingDispatcher::is_sync_needed(SemaphoreSubmitInfo& semaphore_submit) -> bool {
		auto is_sync_needed = sync_needed.load(std::memory_order_relaxed);
		if (is_sync_needed) {
			std::lock_guard lock(semaphore_mutex);
			semaphore_submit = upload_contexts[last_used_uploader_idx]->signal_semaphore;
			sync_needed.store(false, std::memory_order_relaxed);
		}

		return is_sync_needed;
	}

	auto UploadingDispatcher::get_worker_context() -> WorkerContext& {
		static thread_local size_t thread_index = register_thread_context();
		auto& upload_ctx = get_upload_context();
		return *upload_ctx.workers[thread_index];
	}

	auto UploadingDispatcher::get_upload_context() -> UploadContext& {
		return *upload_contexts[active_uploader_idx];
	}

	auto UploadingDispatcher::loop() -> void {
		EDGE_PROFILE_SET_THREAD_NAME("UPLOADING_WORK_DISPATCHER");

		while (!should_stop.load(std::memory_order_relaxed)) {
			// Wait for new jobs
			tf::Taskflow local_jobs{};
			std::vector<std::shared_ptr<AsyncResourceHandle>> local_handles;
			{
				std::unique_lock lock(mutex);
				cv.wait(lock, [&] { return !pending_jobs.empty() || should_stop.load(std::memory_order_relaxed); });

				if (should_stop.load(std::memory_order_relaxed))
					break;

				std::swap(local_jobs, pending_jobs);
				std::swap(pending_handles, local_handles);
			}

			EDGE_PROFILE_ZONE_SCOPED;

#ifndef NDEBUG
			auto start = std::chrono::steady_clock::now();
#endif

			auto& upload_ctx = get_upload_context();

			// Wait previous uploads
			upload_ctx.reset();

#ifndef NDEBUG
			{
				auto tp = std::chrono::steady_clock::now();
				auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(tp - start).count();
				EDGE_LOGT("[UploadingDispatcher] Waiting previous uploads done in {} ms.", wait_time);
				start = tp;
			}
#endif

			EDGE_LOGT("[UploadingDispatcher] Starting working on {} uploads.", local_jobs.num_tasks());

			executor.run(local_jobs).wait();

#ifndef NDEBUG
			{
				auto tp = std::chrono::steady_clock::now();
				auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(tp - start).count();
				EDGE_LOGT("[UploadingDispatcher] All work done in {} ms.", wait_time);
			}
#endif

			// Collect command lists for submit
			std::vector<ICommandList*> cmds{};
			for (auto& worker : upload_ctx.workers)
			{
				if (auto* cmd = worker->end_cmd())
					cmds.emplace_back(cmd);
			}

			// Submit command lists and update resource handles
			if (!cmds.empty())
			{
				std::lock_guard lock(semaphore_mutex);

				// Increment current semaphore
				upload_ctx.increment_semaphore();

				SubmitInfo submit_info{};
				submit_info.command_lists = cmds;
				submit_info.signal_semaphore_infos = { &upload_ctx.signal_semaphore, 1 };
				queue->submit(submit_info, upload_ctx.fence);

				EDGE_LOGT("[UploadingDispatcher] Submitting {} command lists and signalling semaphore with value {}.", cmds.size(), upload_ctx.signal_semaphore.value);

				sync_needed.store(true, std::memory_order_relaxed);

				// Wrap context
				last_used_uploader_idx = active_uploader_idx;
				active_uploader_idx = (active_uploader_idx + 1) % upload_contexts.size();
			}

			for (auto& resource_handle : local_handles)
				resource_handle->done.store(true, std::memory_order_relaxed);

			local_handles.clear();
		}
	}

	auto UploadingDispatcher::register_thread_context() -> size_t {
		std::lock_guard<std::mutex> lock(context_mtx);

		size_t index = next_worker;
		for (auto& upload_ctx : upload_contexts)
			upload_ctx->add_worker(queue, buffer_size);
		++next_worker;

		return index;
	}
}
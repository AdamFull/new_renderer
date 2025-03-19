#include "gfx_resource_uploader.h"

#include "../../util/Logger.h"
#include "../../util/Profiler.h"

#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

#include <chrono>

namespace edge::gfx
{
	WorkerContext::WorkerContext(IContext* gfx_context, const IQueue* queue, uint64_t buffer_size) :
		gfx_context{ gfx_context }
	{
		allocator = std::make_unique<ArenaStagingAllocator>(gfx_context, buffer_size);
		cmd_pool = gfx_context->CreateCommandPool(queue);
	}

	WorkerContext::~WorkerContext()
	{
		gfx_context->Destroy(cmd_pool);
	}

	auto WorkerContext::AcquireCmdList() -> ICommandList*
	{
		if (!recording)
		{
			allocator->Reset();

			cmd_pool->Reset();

			cmd_list = cmd_pool->Allocate();
			cmd_list->Begin();
			//cmd_list->BeginMarker(worker_name, 0xFFFFFFFF);
			recording = true;
		}

		return cmd_list;
	}

	auto WorkerContext::EndCmd() noexcept -> ICommandList*
	{
		ICommandList* out_cmd_list{ nullptr };
		if (recording)
		{
			//cmd_list->EndMarker();
			cmd_list->End();
			out_cmd_list = cmd_list;
			recording = false;
		}

		return out_cmd_list;
	}






	UploadingDispatcher::UploadingDispatcher(IContext* gfx_context, uint64_t buffer_size, uint32_t worker_count) :
		gfx_context{ gfx_context },
		buffer_size{ buffer_size },
		executor{ worker_count }
	{
		//scheduler = utl::Scheduler::Create(worker_count);

		queue = gfx_context->RequestQueue(QueueTypeFlag::eTransfer, QueueTypeFlag::eGraphics | QueueTypeFlag::eCompute);
		queue->SetResourceName("UPLOAD");

		uint32_t uploader_count{ 2 };
		for (int32_t i = 0; i < uploader_count; ++i)
		{
			auto& upload_ctx = upload_contexts.emplace_back();
			upload_ctx.fence = gfx_context->CreateFence();
			upload_ctx.signal_semaphore.semaphore = gfx_context->CreateTimelineSemaphore();
			upload_ctx.signal_semaphore.value = 0;
			upload_ctx.signal_semaphore.stage = StageFlag::eTransfer;
		}
		
		dispatcher_thread = std::jthread([this](std::stop_token stop_token) {Loop(stop_token); });
	}

	UploadingDispatcher::~UploadingDispatcher()
	{
		dispatcher_thread.request_stop();
		cv.notify_all();

		for (auto& upload_ctx : upload_contexts)
		{
			upload_ctx.fence->Wait();
			gfx_context->Destroy(upload_ctx.fence);
			gfx_context->Destroy(upload_ctx.signal_semaphore.semaphore);
		}
	}

	auto UploadingDispatcher::IsSyncNeeded(SemaphoreSubmitInfo& semaphore_submit) -> bool
	{
		auto is_sync_needed = sync_needed.load(std::memory_order_relaxed);
		if (is_sync_needed)
		{
			std::lock_guard lock(semaphore_mutex);
			semaphore_submit = upload_contexts[last_used_uploader_idx].signal_semaphore;
			sync_needed.store(false, std::memory_order_relaxed);
		}

		return is_sync_needed;
	}

	auto UploadingDispatcher::GetWorkerContext() -> WorkerContext&
	{
		static thread_local size_t thread_index = RegisterThreadContext();
		auto& upload_ctx = GetUploadContext();
		return *upload_ctx.workers[thread_index];
	}

	auto UploadingDispatcher::GetUploadContext() -> UploadContext&
	{
		return upload_contexts[active_uploader_idx];
	}

	auto UploadingDispatcher::Loop(std::stop_token stop_token) -> void
	{
		EDGE_PROFILE_SET_THREAD_NAME("UPLOADING_WORK_DISPATCHER");

		while (!stop_token.stop_requested())
		{
			// Wait for new jobs
			tf::Taskflow local_jobs{};
			std::vector<std::shared_ptr<AsyncResourceHandle>> local_handles;
			{
				std::unique_lock lock(mutex);
				cv.wait(lock, [&] { return !pending_jobs.empty() || stop_token.stop_requested(); });

				if (stop_token.stop_requested())
					break;

				std::swap(local_jobs, pending_jobs);
				std::swap(pending_handles, local_handles);
			}

			EDGE_PROFILE_ZONE_SCOPED;

#ifndef NDEBUG
			auto start = std::chrono::steady_clock::now();
#endif

			auto& upload_ctx = GetUploadContext();
			// Wait previous uploads
			upload_ctx.fence->Wait();
			upload_ctx.fence->Reset();

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
				if (auto* cmd = worker->EndCmd())
					cmds.emplace_back(cmd);
			}

			// Submit command lists and update resource handles
			if (!cmds.empty())
			{
				std::lock_guard lock(semaphore_mutex);

				// Increment current semaphore
				++upload_ctx.signal_semaphore.value;

				SubmitInfo submit_info{};
				submit_info.command_lists = cmds;
				submit_info.signal_semaphore_infos = { &upload_ctx.signal_semaphore, 1 };
				queue->Submit(submit_info, upload_ctx.fence);

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

	auto UploadingDispatcher::RegisterThreadContext() -> size_t
	{
		std::lock_guard<std::mutex> lock(context_mtx);

		size_t index = next_worker;
		for (auto& upload_ctx : upload_contexts)
			upload_ctx.workers.emplace_back(std::make_unique<WorkerContext>(gfx_context, queue, buffer_size));
		++next_worker;

		return index;
	}
}
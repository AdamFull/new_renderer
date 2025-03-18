#include "gfx_resource_uploader.h"

#include "../../util/Logger.h"
#include "../../util/Profiler.h"

#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

#include <latch>
#include <chrono>

namespace edge
{
	namespace gfx
	{
		UploadingThreadWorker::UploadingThreadWorker(IContext* gfx_context, const IQueue* queue, uint64_t buffer_size, const std::string& worker_name) :
			gfx_context{ gfx_context },
			worker_name{ worker_name }
		{
			allocator = std::make_unique<ArenaStagingAllocator>(gfx_context, buffer_size);

			cmd_pool = gfx_context->CreateCommandPool(queue);
			thread = std::jthread([this](std::stop_token stop_token) { Loop(stop_token); });
		}

		UploadingThreadWorker::~UploadingThreadWorker()
		{
			thread.request_stop();
			cv.notify_all();

			gfx_context->Destroy(cmd_pool);
		}

		auto UploadingThreadWorker::DispatchJob(ResourceJobPack&& job) -> void
		{
			{
				std::lock_guard lock(mutex);
				jobs.emplace(std::move(job));
				work_count.fetch_add(1, std::memory_order_relaxed);
			}
			cv.notify_one();
		}

		auto UploadingThreadWorker::EndCmd() noexcept -> ICommandList*
		{
			ICommandList* out_cmd_list{ nullptr };
			if (recording)
			{
				cmd_list->EndMarker();
				cmd_list->End();
				out_cmd_list = cmd_list;
				recording = false;
			}

			return out_cmd_list;
		}

		auto UploadingThreadWorker::GetWorkCount() noexcept -> uint64_t
		{
			return work_count.load(std::memory_order_relaxed);
		}

		auto UploadingThreadWorker::AcquireCmdList() -> ICommandList*
		{
			if (!recording)
			{
				allocator->Reset();

				cmd_pool->Reset();

				cmd_list = cmd_pool->Allocate();
				cmd_list->Begin();
				cmd_list->BeginMarker(worker_name, 0xFFFFFFFF);
				recording = true;
			}

			return cmd_list;
		}

		auto UploadingThreadWorker::Loop(std::stop_token stop_token) -> void
		{
			EDGE_PROFILE_SET_THREAD_NAME(worker_name.c_str());

			while (!stop_token.stop_requested())
			{
				std::unique_lock lock(mutex);
				cv.wait(lock, [&] { return !jobs.empty() || stop_token.stop_requested(); });

				if (stop_token.stop_requested())
					break;

				auto job = std::move(jobs.front());
				jobs.pop();
				lock.unlock();

				work_count.fetch_sub(1, std::memory_order_relaxed);

				// Execute the job
				ICommandList* cmd = AcquireCmdList();
				job.task(cmd, *allocator);

				auto result = job.task.get_future().get();
				job.handle->resource = result.first;
				job.handle->state = result.second;
			}
		}

		UploadingDispatcher::UploadingDispatcher(IContext* gfx_context, uint64_t buffer_size, uint32_t worker_count) :
			gfx_context{ gfx_context }
		{
			queue = gfx_context->RequestQueue(QueueTypeFlag::eTransfer, QueueTypeFlag::eGraphics | QueueTypeFlag::eCompute);
			queue->SetResourceName("UPLOAD");

			fence = gfx_context->CreateFence();
			signal_semaphore.semaphore = gfx_context->CreateTimelineSemaphore();
			signal_semaphore.value = 0;
			signal_semaphore.stage = StageFlag::eTransfer;

			for (int32_t i = 0; i < worker_count; ++i)
			{
				auto worker_name = std::format("UPLOADING_THREAD_{}", 1);
				workers.emplace_back(std::make_unique<UploadingThreadWorker>(gfx_context, queue, buffer_size, worker_name));
			}

			dispatcher_thread = std::jthread([this](std::stop_token stop_token) {Loop(stop_token); });
		}

		UploadingDispatcher::~UploadingDispatcher()
		{
			dispatcher_thread.request_stop();
			cv.notify_all();

			fence->Wait();

			gfx_context->Destroy(fence);
			gfx_context->Destroy(signal_semaphore.semaphore);
		}

		void UploadingDispatcher::Submit(ResourceJobPack&& job)
		{
			{
				std::lock_guard lock(mutex);
				pending_jobs.emplace(std::move(job));
			}
			cv.notify_one();
		}

		auto UploadingDispatcher::IsSyncNeeded(SemaphoreSubmitInfo& semaphore_submit) -> bool
		{
			auto is_sync_needed = sync_needed.load(std::memory_order_relaxed);
			if (is_sync_needed)
			{
				std::lock_guard lock(semaphore_mutex);
				semaphore_submit = signal_semaphore;
				sync_needed.store(false, std::memory_order_relaxed);
			}

			return is_sync_needed;
		}

		auto UploadingDispatcher::Loop(std::stop_token stop_token) -> void
		{
			EDGE_PROFILE_SET_THREAD_NAME("UPLOADING_WORK_DISPATCHER");

			while (!stop_token.stop_requested())
			{
				// Wait for new jobs
				std::queue<ResourceJobPack> local_jobs;
				{
					std::unique_lock lock(mutex);
					cv.wait(lock, [&] { return !pending_jobs.empty() || stop_token.stop_requested(); });

					if (stop_token.stop_requested())
						break;

					std::swap(local_jobs, pending_jobs);
				}

				EDGE_PROFILE_ZONE_SCOPED;

#ifndef NDEBUG
				auto start = std::chrono::steady_clock::now();
#endif

				// Wait previous uploads
				fence->Wait();
				fence->Reset();

#ifndef NDEBUG
				{
					auto tp = std::chrono::steady_clock::now();
					auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(tp - start).count();
					EDGE_LOGT("[UploadingDispatcher] Waiting previous uploads done in {} ms.", wait_time);
					start = tp;
				}
#endif

				EDGE_LOGT("[UploadingDispatcher] Starting working on {} uploads.", local_jobs.size());

				auto completion_latch = std::latch(local_jobs.size());

				// Distribute tasks among workers (Round-Robinx)
				while (!local_jobs.empty())
				{
					auto job = std::move(local_jobs.front());
					local_jobs.pop();

					auto original_task = std::make_shared<std::packaged_task<ResourceUploadFnSign>>(std::move(job.task));
					std::packaged_task<ResourceUploadFnSign> wrapped_task(
						[&completion_latch, original_task]
						(ICommandList* cmd, StagingBufferAllocator& alloc) mutable -> ResourceUploadResult {
							(*original_task)(cmd, alloc);
							completion_latch.count_down();
							return original_task->get_future().get();
						}); 
					job.task = std::move(wrapped_task);

					processed_handles.push_back(job.handle);

					size_t selected = 0;
					auto min_size = workers[0]->GetWorkCount();
					for (int64_t i = 1; i < workers.size(); ++i) 
					{
						auto size = workers[i]->GetWorkCount();
						if (size < min_size) 
						{
							min_size = size;
							selected = i;
						}
					}

					//auto& worker = workers[(worker_index++) % workers.size()];
					auto& worker = workers[selected];
					worker->DispatchJob(std::move(job));
				}

				// Wait until all workers finish their tasks
				completion_latch.wait();

#ifndef NDEBUG
				{
					auto tp = std::chrono::steady_clock::now();
					auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(tp - start).count();
					EDGE_LOGT("[UploadingDispatcher] All work done in {} ms.", wait_time);
				}
#endif

				// Collect command lists for submit
				std::vector<ICommandList*> cmds{};
				for (auto& worker : workers)
				{
					if (auto* cmd = worker->EndCmd())
						cmds.emplace_back(cmd);
				}

				// Submit command lists and update resource handles
				if (!cmds.empty())
				{
					std::lock_guard lock(semaphore_mutex);

					// Increment current semaphore
					++signal_semaphore.value;

					SubmitInfo submit_info{};
					submit_info.command_lists = cmds;
					submit_info.signal_semaphore_infos = { &signal_semaphore, 1 };
					queue->Submit(submit_info, fence);

					EDGE_LOGT("[UploadingDispatcher] Submitting {} command lists and signalling semaphore with value {}.", cmds.size(), signal_semaphore.value);

					sync_needed.store(true, std::memory_order_relaxed);
				}

				for (auto& resource_handle : processed_handles)
					resource_handle->done.store(true, std::memory_order_relaxed);

				processed_handles.clear();
			}
		}
	}
}
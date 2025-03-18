#include "gfx_resource_uploader.h"

#include "../util/Logger.h"

#include "gfx_staging_allocator.h"

#include "../gfx_context.h"

namespace edge
{
	namespace gfx
	{
		UploadingThreadWorker::UploadingThreadWorker(IContext* gfx_context, const IQueue* queue, uint64_t buffer_size) :
			gfx_context{ gfx_context }
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
			}
			cv.notify_one();
		}

		auto UploadingThreadWorker::EndCmd() -> ICommandList*
		{
			ICommandList* out_cmd_list{ nullptr };
			if (recording)
			{
				cmd_list->End();
				out_cmd_list = cmd_list;
				recording = false;
			}

			return out_cmd_list;
		}

		auto UploadingThreadWorker::IsIdle() -> bool
		{
			return is_idle.load(std::memory_order_relaxed);
		}

		auto UploadingThreadWorker::AcquireCmdList() -> ICommandList*
		{
			if (!recording)
			{
				allocator->Reset();

				cmd_pool->Reset();

				cmd_list = cmd_pool->Allocate();
				cmd_list->Begin();
				recording = true;
			}

			return cmd_list;
		}

		auto UploadingThreadWorker::Loop(std::stop_token stop_token) -> void
		{
			while (!stop_token.stop_requested())
			{
				std::unique_lock lock(mutex);
				cv.wait(lock, [&] { return !jobs.empty() || stop_token.stop_requested(); });

				if (stop_token.stop_requested())
					break;

				is_idle.store(false, std::memory_order_relaxed);

				auto job = std::move(jobs.front());
				jobs.pop();
				lock.unlock();

				ICommandList* cmd = AcquireCmdList();

				// Execute the job
				job.task(cmd, *allocator);

				auto future = job.task.get_future();
				auto result = future.get();

				job.handle->resource = result.first;
				job.handle->state = result.second;

				if (jobs.empty())
				{
					is_idle.store(true, std::memory_order_relaxed);
				}
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
				workers.emplace_back(std::make_unique<UploadingThreadWorker>(gfx_context, queue, buffer_size));
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
				semaphore_mutex.lock();
				semaphore_submit = signal_semaphore;
				semaphore_mutex.unlock();
				sync_needed.store(false, std::memory_order_relaxed);
			}

			return is_sync_needed;
		}

		auto UploadingDispatcher::Loop(std::stop_token stop_token) -> void
		{
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

				// Wait previous uploads
				fence->Wait();
				fence->Reset();

				EDGE_LOGD("[ResourceUpdater] Starting working on {} uploads.", local_jobs.size());

				// Distribute tasks among workers (Round-Robin)
				while (!local_jobs.empty())
				{
					auto& worker = workers[worker_index % workers.size()];
					auto job = std::move(local_jobs.front());

					processed_handles.push_back(job.handle);
					worker->DispatchJob(std::move(job));

					local_jobs.pop();
					++worker_index;
				}

				// Wait until all workers finish their tasks
				bool all_done;
				do
				{
					all_done = true;

					// avoid busy-waiting aggressively
					std::this_thread::sleep_for(std::chrono::microseconds(10));

					for (auto& worker : workers)
					{
						if (!worker->IsIdle())
						{
							all_done = false;
							break;
						}
					}
				} while (!all_done);

				// Collect command lists for submit
				std::vector<ICommandList*> command_lists{};
				for (auto& worker : workers)
				{
					auto* cmd_list = worker->EndCmd();
					if (cmd_list)
						command_lists.emplace_back(cmd_list);
				}

				// Increment current semaphore
				semaphore_mutex.lock();
				++signal_semaphore.value;
				semaphore_mutex.unlock();

				// Submit command lists and update resource handles
				if (!command_lists.empty())
				{
					SubmitInfo submit_info{};
					submit_info.command_lists = command_lists;
					submit_info.signal_semaphore_infos = { &signal_semaphore, 1 };
					//submit_info.wait_semaphore_infos = wait_semaphores;
					queue->Submit(submit_info, fence);

					sync_needed.store(true, std::memory_order_relaxed);

					for (auto& resource_handle : processed_handles)
						resource_handle->done.store(true, std::memory_order_relaxed);

					processed_handles.clear();
				}
			}
		}
	}
}
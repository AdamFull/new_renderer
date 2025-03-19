#include "utl_thread_pool.h"

namespace edge::utl
{
	ThreadPool::ThreadPool(size_t num_threads) :
		done{ false }, next_worker{ 0 }, tasks_remaining{ 0 }
	{
		for (int32_t i = 0; i < num_threads; ++i) 
		{
			workers.emplace_back(std::make_unique<WorkerCtx>());
			threads.emplace_back([this, i](std::stop_token stop_token) { WorkerLoop(stop_token, i); });
		}
	}

	ThreadPool::~ThreadPool()
	{
		done.store(true, std::memory_order_release);
		cv.notify_all();

		for (auto& thread : threads)
			thread.request_stop();
	}

	void ThreadPool::WorkerLoop(std::stop_token stop_token, size_t worker_idx)
	{
		while (!stop_token.stop_requested())
		{
			std::function<void()> task;

			if (PopTaskFromWorker(worker_idx, task) || StealTask(worker_idx, task))
			{
				task();

				if (tasks_remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) 
				{
					std::lock_guard<std::mutex> lock(tasks_mtx);
					tasks_cv.notify_all();
				}
			}
			else 
			{
				// No task found: wait a bit for new tasks.
				std::unique_lock<std::mutex> lock(cv_mtx);
				cv.wait_for(lock, std::chrono::milliseconds(1));
			}
		}
	}

	bool ThreadPool::PopTaskFromWorker(size_t index, std::function<void()>& task)
	{
		std::lock_guard<std::mutex> lock(workers[index]->mtx);
		if (!workers[index]->tasks.empty()) 
		{
			task = std::move(workers[index]->tasks.back());
			workers[index]->tasks.pop_back();
			return true;
		}
		return false;
	}

	bool ThreadPool::StealTask(size_t thief_index, std::function<void()>& task)
	{
		size_t num_workers = workers.size();
		for (int32_t i = 0; i < num_workers; ++i) 
		{
			// Steal from the next worker in the ring.
			size_t victim = (thief_index + i + 1) % num_workers;
			std::lock_guard<std::mutex> lock(workers[victim]->mtx);
			if (!workers[victim]->tasks.empty())
			{
				task = std::move(workers[victim]->tasks.front());
				workers[victim]->tasks.pop_front();
				return true;
			}
		}
		return false;
	}
}
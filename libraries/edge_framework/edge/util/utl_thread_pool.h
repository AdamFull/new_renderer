#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <utility>

namespace edge
{
	namespace utl
	{
		class ThreadPool
		{
		public:
			explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
			~ThreadPool();

			template<typename _F, typename... _Args>
			auto Submit(_F&& f, _Args&&... args) -> std::future<std::invoke_result_t<_F, _Args...>>
			{
				using result_type = std::invoke_result_t<_F, _Args...>;

				auto task = std::make_shared<std::packaged_task<result_type()>>(
					std::bind(std::forward<_F>(f), std::forward<_Args>(args)...)
				);

				std::future<result_type> res = task->get_future();
				auto wrapper = [task]() { (*task)(); };

				tasks_remaining.fetch_add(1, std::memory_order_relaxed);

				size_t index = next_worker.fetch_add(1, std::memory_order_relaxed) % workers.size();
				{
					std::lock_guard<std::mutex> lock(workers[index]->mtx);
					workers[index]->tasks.push_back(std::move(wrapper));
				}
				// Notify one waiting worker that a new task is available.
				cv.notify_one();

				return res;
			}
		private:
			void WorkerLoop(std::stop_token stop_token, size_t worker_idx);
			bool PopTaskFromWorker(size_t index, std::function<void()>& task);
			bool StealTask(size_t thief_index, std::function<void()>& task);

			struct WorkerCtx
			{
				std::deque<std::function<void()>> tasks;
				std::mutex mtx;
			};

			std::vector<std::unique_ptr<WorkerCtx>> workers;
			std::vector<std::jthread> threads;

			std::atomic_bool done;
			std::atomic_size_t next_worker;

			std::condition_variable cv;
			std::mutex cv_mtx;

			std::atomic_size_t tasks_remaining;
			std::condition_variable tasks_cv;
			std::mutex tasks_mtx;
		};
	}
}
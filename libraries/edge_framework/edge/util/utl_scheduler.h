// https://poniesandlight.co.uk/reflect/coroutines_job_system/
// https://github.com/tgfrerer/pal_tasks/

#pragma once

#include <coroutine>
#include <cstdint>

namespace edge
{
	namespace utl
	{
		struct TaskPromise;

		struct Task : std::coroutine_handle<TaskPromise> 
		{
			using promise_type = TaskPromise;
		};

		struct suspend_task 
		{
			// if await_ready is false, then await_suspend will be called
			constexpr bool await_ready() noexcept { return false; };
			void await_suspend(std::coroutine_handle<TaskPromise> h) noexcept;
			void await_resume() noexcept {};
		};

		struct finalize_task 
		{
			// if await_ready is false, then await_suspend will be called
			constexpr bool await_ready() noexcept { return false; };
			void await_suspend(std::coroutine_handle<TaskPromise> h) noexcept;
			void await_resume() noexcept {};
		};

		class scheduler_impl;
		class task_list_o;
		class TaskList;

		class Scheduler 
		{
			scheduler_impl* p_impl = nullptr;

			Scheduler(int32_t num_worker_threads);

			Scheduler(const Scheduler&) = delete;
			Scheduler(Scheduler&&) = delete;
			Scheduler& operator=(const Scheduler&) = delete;
			Scheduler& operator=(Scheduler&&) = delete;

		public:
			// Execute all tasks in the task list, then free the task list object
			// this takes possession of the task list object, and acts as if it was
			// a blocking call.
			//
			// Once this call returns, the TaskList that was given as a parameter
			// has been consumed, and you should not re-use it.
			void ExecuteList(TaskList& p_t);

			// Create a scheduler with as many hardware threads as possible
			//  0 ... No worker threads, just one main thread
			//  n ... n number of worker threads
			// -1 ... As many worker threads as cpus, -1
			static Scheduler* Create(int32_t num_worker_threads = 0);

			~Scheduler();
		};

		class TaskList 
		{
			task_list_o* p_impl; // owning

			TaskList(const TaskList&) = delete;
			TaskList(TaskList&&) = delete;
			TaskList& operator=(const TaskList&) = delete;
			TaskList& operator=(TaskList&&) = delete;

		public:
			TaskList(uint32_t hint_capacity = 1);

			~TaskList();

			void PushTask(Task c);

			friend class scheduler_impl;
		};

		struct TaskPromise 
		{
			Task get_return_object() 
			{
				return { Task::from_promise(*this) };
			}

			std::suspend_always initial_suspend() noexcept 
			{
				return {};
			}

			finalize_task   final_suspend() noexcept { return {}; }
			void            return_void() {};
			void            unhandled_exception() {};
			scheduler_impl* scheduler = nullptr;
			task_list_o* p_task_list = nullptr;
		};
	}
}
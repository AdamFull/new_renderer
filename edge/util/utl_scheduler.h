#pragma once

#include <coroutine>
#include <exception>

namespace edge
{
	namespace utl
	{
		class SchedulerImpl;
		class TaskListImpl;
		class TaskList;

		struct Job
		{
			std::coroutine_handle<> handle;
			Job(std::coroutine_handle<> h) : handle(h) {}
			void operator()() { handle.resume(); }
			static Job from_address(void* addr)
			{
				return Job(std::coroutine_handle<>::from_address(addr));
			}
		};

		struct TaskPromiseBase
		{
			TaskListImpl* task_list = nullptr;
			SchedulerImpl* scheduler = nullptr;
			std::coroutine_handle<> continuation = nullptr;
			std::exception_ptr exception;
		};

		struct final_awaitable
		{
			bool await_ready() noexcept { return false; }
			std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept;
			void await_resume() noexcept {}
		};

		struct suspend_task
		{
			constexpr bool await_ready() noexcept { return false; }
			void await_suspend(std::coroutine_handle<> h) noexcept;
			void await_resume() noexcept {}
		};

		template<typename _T = void>
		struct Task
		{
			struct promise_type : public TaskPromiseBase
			{
				_T result;

				auto get_return_object() { return Task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
				std::suspend_always initial_suspend() noexcept { return {}; }
				auto final_suspend() noexcept { return final_awaitable{}; }
				void return_value(_T value) { result = std::move(value); }

				void unhandled_exception() { exception = std::current_exception(); }
			};

			std::coroutine_handle<promise_type> coro;

			void operator()() { coro.resume(); }

			Task(std::coroutine_handle<promise_type> h) : coro(h) {}
			Task(const Task&) = delete;
			Task(Task&& other) : coro(other.coro) { other.coro = nullptr; }
			Task& operator=(Task&& other)
			{
				if (this != &other)
				{
					if (coro)
						coro.destroy();
					coro = other.coro;
					other.coro = nullptr;
				}
				return *this;
			}

			bool await_ready() const noexcept { return false; }
			auto await_suspend(std::coroutine_handle<> cont)
			{
				coro.promise().continuation = cont;
				return coro;
			}

			auto await_resume()
			{
				if (coro.promise().exception)
					std::rethrow_exception(coro.promise().exception);
				if constexpr (!std::is_void_v<_T>)
					return std::move(coro.promise().result);
			}

			// Allow conversion to a type–erased Job (for the scheduler)
			operator Job() { return Job(coro); }

			// Helpers so that a Task can be constructed from a void* stored in the task list.
			static Task from_address(void* addr) 
			{
				return Task(std::coroutine_handle<promise_type>::from_address(addr));
			}
		};

		template<>
		struct Task<void>::promise_type : public TaskPromiseBase
		{
			auto get_return_object() { return Task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
			std::suspend_always initial_suspend() noexcept { return {}; }
			auto final_suspend() noexcept { return final_awaitable{}; }

			void return_void() {}
			void unhandled_exception() { exception = std::current_exception(); }
		};

		class Scheduler
		{
		public:
			~Scheduler();

			static Scheduler* Create(int32_t num_worker_threads = 0);

			void ExecuteList(TaskList& t);
		private:
			SchedulerImpl* p_impl = nullptr;

			Scheduler(int32_t num_worker_threads);

			Scheduler(const Scheduler&) = delete;
			Scheduler(Scheduler&&) = delete;
			Scheduler& operator=(const Scheduler&) = delete;
			Scheduler& operator=(Scheduler&&) = delete;
		};

		class TaskList
		{
		public:
			friend class SchedulerImpl;

			TaskList(uint32_t hint_capacity = 1);
			~TaskList();

			void AddTask(Job c);

		private:
			TaskListImpl* p_impl;
		};
	}
}
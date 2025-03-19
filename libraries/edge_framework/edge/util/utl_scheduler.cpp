#include "utl_scheduler.h"

#include <iostream>

#include "lockfree_ring_buffer.h"

namespace edge::utl
{
	// A channel is a thread-safe primitive to communicate with worker threads -
	// each worker thread has exactly one channel. Once a channel contains
	// a payload it is blocked (you cannot push anymore handles onto this channel).
	// The channel gets free and ready to receive another handle as soon as the
	// worker thread has finished processing the current handle.
	struct Channel
	{
		void* handle; // storage for channel payload: one single handle. void means that the channel is free.
		std::atomic_flag flag;   // signal that the current channel is busy.

		bool TryPush(std::coroutine_handle<>& h)
		{

			if (flag.test_and_set())
			{
				// if the current channel was already flagged
				// we cannot add anymore work.
				return false;
			}

			// --------| invariant: current channel is available now

			handle = h.address();

			// If there is a thread blocked on this operation, we
			// unblock it here.
			flag.notify_one();

			return true;
		}

		~Channel()
		{

			// Once the channel accepts a coroutine handle, it becomes the
			// owner of the handle. If there are any leftover valid handles
			// that we own when this object dips into ovlivion, we must clean
			// them up first.
			//
			if (this->handle)
			{
				// std::cout << "WARNING: leftover task in channel." << std::endl;
				// std::cout << "destroying task: " << this->handle << std::endl;
				Job::from_address(handle).handle.destroy();
			}

			this->handle = nullptr;
		}
	};

	class TaskListImpl
	{
		lockfree_ring_buffer_t tasks;
		std::atomic_size_t     num_tasks; // number of tasks, only gets decremented if taks has been removed

	public:
		std::atomic_flag block_flag; // flag used to signal that dependent tasks have completed

		TaskListImpl(uint32_t capacity_hint = 32) // start with capacity of 32
			: tasks(capacity_hint)
			, num_tasks(0)
		{
		}

		~TaskListImpl()
		{
			// If there are any tasks left on the task list, we must destroy them, as we own them.
			void* task;
			while ((task = this->tasks.try_pop()))
			{
				Job::from_address(task).handle.destroy();
			}
		}

		// Push a suspended task back onto the end of the task list
		inline void PushTask(std::coroutine_handle<> const& c)
		{
			tasks.push(c.address());
		}

		// Get the next task if possible, if there is no next task,
		// return an empty coroutine handle.
		// An empty coroutine handle will compare true to nullptr
		inline Job PopTask()
		{
			void* ptr = tasks.try_pop();
			if (ptr)
				return Job::from_address(ptr);

			return Job(std::noop_coroutine());
		}

		// Return the number of tasks which are both in flight and waiting
		//
		// Note this is not the same as tasks.size() as any tasks which are being
		// processed and are in flight will not show up on the task list.
		//
		// num_tasks gets decremented only if a task was fully completed.
		inline size_t GetTasksCount()
		{
			return num_tasks;
		}

		// Add a new task to the task list - only allowed in setup phase,
		// where only one thread has access to the task list.
		void AddTask(std::coroutine_handle<TaskPromiseBase> c)
		{
			c.promise().task_list = this;
			tasks.unsafe_initial_dynamic_push(c.address());
			num_tasks++;
		}

		void TagAllTasksWithScheduler(SchedulerImpl* p_scheduler)
		{
			tasks.unsafe_for_each(
				[](void* c, void* p_scheduler) {
					auto h = std::coroutine_handle<TaskPromiseBase>::from_address(c);
					h.promise().scheduler = static_cast<SchedulerImpl*>(p_scheduler);
				},
				p_scheduler);
		}

		void DecrementTaskCount()
		{
			size_t num_flags = --num_tasks;
			if (num_flags == 0)
			{
				block_flag.clear(std::memory_order_release);
				block_flag.notify_one(); // unblock us on block flag.
			}
		}
	};

	TaskList::TaskList(uint32_t hint_capacity)
		: p_impl(new TaskListImpl(hint_capacity))
	{
	}

	void TaskList::AddTask(Job j)
	{
		assert(p_impl != nullptr && "task list must be valid. Was this task list already used?");
		auto h = std::coroutine_handle<TaskPromiseBase>::from_address(j.handle.address());
		p_impl->AddTask(h);
	}

	TaskList::~TaskList()
	{
		// In case that this task list was deleted already, p_impl will
		// be nullptr, which means that this delete operator is a no-op.
		// otherwise (in case a tasklist has not been used and needs to
		// be cleaned up), this will perform the cleanup for us.
		delete p_impl;
	}

	class SchedulerImpl
	{

		bool MoveTaskToWorkerThread(std::coroutine_handle<>& c);

	public:
		std::vector<Channel*>     channels; // non-owning - channels are owned by their threads
		std::vector<std::jthread> threads;

		SchedulerImpl(int32_t num_worker_threads = 0);

		~SchedulerImpl()
		{
			// We must unblock any threads which are currently waiting on a flag signal for more work
			// as there is no more work coming, we must artificially signal the flag so that these
			// worker threads can resume to completion.
			for (auto* c : channels)
			{
				if (c)
				{
					c->flag.test_and_set(); // Set flag so that if there is a worker blocked on this flag, it may proceed.
					c->flag.notify_one();   // Notify the worker thread (if any worker thread is waiting) that the flag has flipped.
					// without notify, waiters will not be notified that the flag has flipped.
				}
			}
			// We must wait until all the threads have been joined.
			// Deleting a jthread object implicitly stops (sets the stop_token) and joins.
			threads.clear();
		}

		// Execute all tasks in the task list, then invalidate the task list object
		void WaitForTaskList(TaskList& p_t);
	};

	SchedulerImpl::SchedulerImpl(int32_t num_worker_threads)
	{
		if (num_worker_threads < 0)
		{
			// If negative, then this means that we must
			// count backwards from the number of available hardware threads
			num_worker_threads = std::jthread::hardware_concurrency() + num_worker_threads;
		}

		assert(num_worker_threads >= 0 && "Inferred number of worker threads must not be negative");

		// std::cout << "Initializing scheduler with " << num_worker_threads << " worker threads" << std::endl;

		// Reserve memory so that we can take addresses for channel,
		// and don't have to worry about iterator validity
		channels.reserve(num_worker_threads);
		threads.reserve(num_worker_threads);

		// NOTE THAT BY DEFAULT WE DON'T HAVE ANY WORKER THREADS
		//
		for (int i = 0; i != num_worker_threads; i++)
		{
			channels.emplace_back(new Channel());
			threads.emplace_back(
				//
				// Thread worker implementation
				//
				[](std::stop_token stop_token, Channel* ch)
				{
					while (!stop_token.stop_requested())
					{

						if (ch->handle)
						{
							std::coroutine_handle<>::from_address(ch->handle).resume();// resume coroutine
							ch->handle = nullptr;
							// signal that we are ready to receive new tasks
							ch->flag.clear(std::memory_order::release);
							continue;
						}

						// Wait for flag to be set
						//
						// The flag is set on any of the following:
						//   * A new job has been placed in the channel
						// 	 * The current task list is empty.
						ch->flag.wait(false, std::memory_order::acquire);
					}

					// Channel is owned by the thread - when the thread falls out of scope
					// that means that the channel gets deleted, too.
					delete ch;
				},
				channels.back());
		}
	}

	void SchedulerImpl::WaitForTaskList(TaskList& p_t)
	{

		if (p_t.p_impl == nullptr) {
			assert(false && "Task list must have been freshly initialised. Has this task list been waited for already?");
			return;
		}

		// --------| Invariant: TaskList is valid

		// Execute tasks in this task list until there are no more tasks left
		// to execute.

		// Before we start executing tasks we must take ownership of them
		// by tagging them so that they know which scheduler they belong to.
		p_t.p_impl->TagAllTasksWithScheduler(this);

		// Distribute work, as long as there is work to distribute

		while (p_t.p_impl->GetTasksCount())
		{

			// ----------| Invariant: There are Tasks in this Task List which have not yet completed

			Job j = (p_t.p_impl)->PopTask();

			if (!j.handle)
			{
				// We could not fetch a task from the task list - this means
				// that there are tasks in-progress that we must wait for.

				if (p_t.p_impl->block_flag.test_and_set(std::memory_order::acq_rel))
				{
					std::cout << "blocking thread " << std::this_thread::get_id() << " on [" << p_t.p_impl << "]" << std::endl;
					// Wait for the flag to be set - this is the case if any of these happen:
					//    * the scheduler is destroyed
					//    * the last task of the task list has completed, and the task list is now empty.
					p_t.p_impl->block_flag.wait(true, std::memory_order::acquire);
					std::cout << "resuming thread " << std::this_thread::get_id() << " on [" << p_t.p_impl << "]" << std::endl;
				}
				else
				{
					std::cout << "spinning thread " << std::this_thread::get_id() << " on [" << p_t.p_impl << "]" << std::endl;
				}

				continue;
			}

			// ----------| Invariant: current coroutine is valid

			// Find a free channel. if there is, then place this handle in the channel,
			// which means that it will be executed on the worker thread associated
			// with this channel.

			if (MoveTaskToWorkerThread(j.handle))
			{
				// Pushing consumes the coroutine handle - that is, it becomes owned by the channel
				// who owns it for the worker thread.
				//
				// If we made it in here, the handle was successfully offloaded to a worker thread.
				//
				// The worker thread must now execute the payload, and the task will decrement the
				// counter for the current TaskList upon completion.
				continue;
			}

			// --------| Invariant: All worker threads are busy - or there are no worker threads: we must execute on this thread
			j();
		}

		// Once all tasks have been complete, release task list
		delete p_t.p_impl;    // Free task list impl
		p_t.p_impl = nullptr; // Signal to any future users that this task list has been used already
	}

	// ----------------------------------------------------------------------

	inline bool SchedulerImpl::MoveTaskToWorkerThread(std::coroutine_handle<>& c)
	{
		// Iterate over all channels. If we can place the coroutine
		// on a channel, do so.
		for (auto& ch : channels)
		{
			if (true == ch->TryPush(c))
			{
				return true;
			}
		}
		return false;
	}

	// ----------------------------------------------------------------------

	Scheduler::Scheduler(int32_t num_worker_threads)
		: p_impl(new SchedulerImpl(num_worker_threads)) 
	{
	}

	void Scheduler::ExecuteList(TaskList& p_t)
	{
		p_impl->WaitForTaskList(p_t);
	}

	Scheduler* Scheduler::Create(int32_t num_worker_threads)
	{
		return new Scheduler(num_worker_threads);
	}

	Scheduler::~Scheduler()
	{
		delete p_impl;
	}



	std::coroutine_handle<> final_awaitable::await_suspend(std::coroutine_handle<> h) noexcept
	{
		auto h_base = std::coroutine_handle<TaskPromiseBase>::from_address(h.address());
		auto& promise = h_base.promise();
		promise.task_list->DecrementTaskCount();
		if (promise.continuation)
		{
			// Grab the continuation and clear it.
			auto cont = promise.continuation;
			promise.continuation = nullptr;
			return cont;
		}
		return std::noop_coroutine();
	}

	void suspend_task::await_suspend(std::coroutine_handle<> h) noexcept
	{
		auto h_base = std::coroutine_handle<TaskPromiseBase>::from_address(h.address());
		auto& promise = h_base.promise();
		promise.task_list->PushTask(h_base);
		// Wake up the scheduler in case a worker thread is waiting.
		promise.task_list->block_flag.clear(std::memory_order_release);
		promise.task_list->block_flag.notify_one();
		// Optionally, eagerly fetch the next job.
		Job next = promise.task_list->PopTask();
		if (next.handle)
		{
			next();
		}
	}
}
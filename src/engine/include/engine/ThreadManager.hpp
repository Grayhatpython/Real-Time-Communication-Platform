#pragma once
#include "ThreadRole.hpp"

namespace engine
{
	enum class TaskStatus
	{
		Created,
		Running,
		Completed,
		Canceled,
		Failed
	};

	using CallbackFunc = std::function<void()>;

	class Task
	{

	public:
		Task(CallbackFunc func, const std::string& name = "");
		virtual ~Task() = default;

	public:
		void Execute();
		bool Cancel();

	private:
		bool TryStart();

	public:
		const std::string& GetName() const { return _name; }
		TaskStatus GetStatus() const { return _status.load(std::memory_order_relaxed); }
		bool IsDone() const;

	private:
		std::string					_name;
		CallbackFunc				_func;
		std::atomic<TaskStatus>		_status = TaskStatus::Created;
		std::atomic<bool>			_cancelRequested = false;
	};

	class TaskQueue
	{
	public:
		void 					Push(std::shared_ptr<Task> task);
		std::shared_ptr<Task>	Push(CallbackFunc func, const std::string& name = "");

		std::shared_ptr<Task> Pop();

		bool IsEmpty() const;
		void Clear();
		void NotifyOne();
		void NotifyAll();
		void Shutdown();

		size_t Size() const;

	private:
		mutable std::mutex _lock;
		std::queue<std::shared_ptr<Task>> _tasks;
		std::condition_variable _cv;
		bool _shutdown = false;
	};

	using Event = Task;
	using EventQueue = TaskQueue;

	class ThreadManager
	{
	public:
		ThreadManager();
		~ThreadManager();
		
	public:
		using ExitCallback = std::function<void()>;
    	using ThreadFunc = std::function<void(std::stop_token)>;

		struct CallbackEntry
		{
			ThreadRole mask;
			ExitCallback callback;
			std::string name;
		};

		struct ThreadHandle
		{
			size_t id = static_cast<size_t>(-1);
		};

	public:
		void RegisterExitCallback(ThreadRole mask, ExitCallback callback, std::string name = {});
  		ThreadHandle Spawn(std::string name, ThreadRole role, ThreadFunc func);

	public:
		void RequestStop(ThreadHandle handle); 
		void Join(ThreadHandle handle);
		void StopAllAndJoin();

	private:
		struct ManagedThread
		{
			std::string name;
			ThreadRole role = ThreadRole::None;
			std::jthread thread;
		};

		void RunExitCallback(ThreadRole role);

	private:
	 	std::mutex 					_lock;
    	std::vector<CallbackEntry> 	_callbackEntrys;

    	std::vector<ManagedThread> _threads;
	};
}
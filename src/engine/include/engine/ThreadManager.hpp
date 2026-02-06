#pragma once

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
		ThreadManager(int32 threadCount = 0);
		~ThreadManager();

	public:
		void Launch(std::function<void()> callback, const std::string& threadName = "", bool repeated = false);
		void Join();
		void Stop();

	public:
		void InitializeThreadPool(int32 threadCount = 0);
		void ShutdownThreadPool();

		//	Push Task No return Value
		std::shared_ptr<Task> PushTask(std::function<void(void)> func, const std::string& name);

	public:
		static void InitializeThreadLocal(const std::string& name);
		static void DestroyThreadLocal();
		static void RegisterDestroyThreadLocal(std::function<void(void)> destroyTLSCallback);

		static void 		SetCurrentThreadName(const std::string& name);
		static std::string 	GetCurrentThreadName();

	private:
		void WorkerThread();

	private:
		std::mutex _lock;
		std::vector<std::thread> _threads;
		std::atomic<bool> _stopped = false;

		std::vector<std::thread> 		_threadPool;
		std::shared_ptr<TaskQueue> 		_taskQueue;
		std::atomic<bool>				_poolRunning = false;
	};
}
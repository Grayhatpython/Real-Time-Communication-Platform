#include "engine/ThreadManager.hpp"
#include "engine/MemoryPool.hpp"

namespace engine
{
	Task::Task(CallbackFunc func, const std::string& name)
		: _name(std::move(name)), _func(std::move(func))
	{

	}

	void Task::Execute()
	{
		// 이미 취소되었거나, 이미 실행/완료됨
		if(TryStart() == false)
			return;

		// 2) 실행 직전 취소 확인
		if(_cancelRequested.load(std::memory_order_relaxed) == true)
		{
			_status.store(TaskStatus::Canceled, std::memory_order_release);
			return;
		}

		//	실행
		if(_func)
		{
			_func();
			_status.store(TaskStatus::Completed, std::memory_order_release);
		}
		else
		{
			//	일감이 없는데 실행?
			_status.store(TaskStatus::Failed, std::memory_order_release);
		}
	}

	// Created 상태면 "확정 취소" (Created -> Canceled)
    // 이미 Running 이후면 "요청만" 걸고 false 반환
	bool Task::Cancel()
	{
		_cancelRequested.store(true, std::memory_order_relaxed);

        TaskStatus expected = TaskStatus::Created;
        if (_status.compare_exchange_strong(
                expected, TaskStatus::Canceled,
                std::memory_order_relaxed,
                std::memory_order_relaxed))
        {
            return true; // 실행 전에 취소 확정
        }

        return false; // 이미 시작했거나 완료됨(확정 취소 불가)
	}

    bool Task::TryStart()
    {
		TaskStatus expected = TaskStatus::Created;

		 // 1) 실행 권한 획득: Created -> Running
		return _status.compare_exchange_strong(expected, TaskStatus::Running, 
			std::memory_order_relaxed, std::memory_order_relaxed);
    }

    bool Task::IsDone() const
    {
     	auto status = GetStatus();
        return status == TaskStatus::Completed || status == TaskStatus::Canceled || status == TaskStatus::Failed;
    }

    void TaskQueue::Push(std::shared_ptr<Task> task)
	{
		{
			std::lock_guard<std::mutex> lock(_lock);
			if (_shutdown)
				return;

			_tasks.push(task);
		}

		_cv.notify_one();
	}

	std::shared_ptr<Task> TaskQueue::Push(CallbackFunc func, const std::string& name)
	{
		auto task = std::make_shared<Task>(std::move(func), std::move(name));

		{
			std::lock_guard<std::mutex> lock(_lock);
			if (_shutdown)
				return nullptr;

			_tasks.push(task);
		}

		_cv.notify_one();
		return task;
	}

	std::shared_ptr<Task> TaskQueue::Pop()
	{
		std::unique_lock<std::mutex> lock(_lock);
		_cv.wait(lock, [this]() { return _shutdown || _tasks.empty() == false; });

		if (_shutdown || _tasks.empty() == true)
			return nullptr;

		auto task = _tasks.front();
		_tasks.pop();
		return task;
	}

	bool TaskQueue::IsEmpty() const
	{
		std::lock_guard<std::mutex> lock(_lock);
		return _tasks.empty();
	}

	void TaskQueue::Clear()
	{
	    std::lock_guard<std::mutex> lock(_lock);
		std::queue<std::shared_ptr<Task>> empty;
		_tasks.swap(empty);
	}

	void TaskQueue::NotifyOne()
	{
		_cv.notify_one();
	}

	void TaskQueue::NotifyAll()
	{
		_cv.notify_all();
	}

	void TaskQueue::Shutdown()
	{
		{
			std::lock_guard<std::mutex> lock(_lock);
			_shutdown = true;
		}

		_cv.notify_all();
	}

	size_t TaskQueue::Size() const
	{
		std::lock_guard<std::mutex> lock(_lock);
		return _tasks.size();
	}

	ThreadManager::ThreadManager(int32 threadCount)
	{
		_taskQueue = std::make_shared<TaskQueue>();

		InitializeThreadLocal("Main");

		if (threadCount > 0)
			InitializeThreadPool();
	}

	ThreadManager::~ThreadManager()
	{

	}

	void ThreadManager::Launch(std::function<void()> callback, const std::string& threadName,  bool repeated)
	{
		if (_stopped.load(std::memory_order_acquire) == true)
			return;

		std::lock_guard<std::mutex> lock(_lock);

		_threads.push_back(std::thread([this, callback, threadName, repeated]() {
			
			InitializeThreadLocal(threadName);

            EN_LOG_INFO("Thread Started");

			if(repeated == true)
			{
				while(_stopped.load(std::memory_order_acquire) == false)
					callback();
			}
			else
				callback();

			DestroyThreadLocal();

			EN_LOG_INFO("Thread finished");
		}));
	}

	void ThreadManager::Join()
	{
		for (std::thread& thread : _threads)
		{
			if (thread.joinable())
				thread.join();
		}
		_threads.clear();

		EN_LOG_INFO("Threads Join finished");
	}

	void ThreadManager::Stop()
	{
		EN_LOG_INFO("ThreadManager Stopping");

		_stopped.store(true, std::memory_order_release);

		Join();

		//	Thread Pool은 아직...
		ShutdownThreadPool();

		//	Main Thread
		DestroyThreadLocal();

		EN_LOG_INFO("ThreadManager Stopped");
	}

	void ThreadManager::InitializeThreadPool(int32 threadCount)
	{
		if (_poolRunning.load(std::memory_order_acquire) == true)
			return;

		if (threadCount <= 0)
			threadCount = std::thread::hardware_concurrency();

		_poolRunning.store(true, std::memory_order_release);

		for (int32 i = 0; i < threadCount; i++)
		{
			_threadPool.push_back(std::thread([this, i]() {
					WorkerThread();
				}));
		}
	}

	void ThreadManager::ShutdownThreadPool()
	{
		_poolRunning.store(true, std::memory_order_release);
		_taskQueue->Shutdown();

		for (auto& thread : _threadPool)
		{
			if (thread.joinable())
				thread.join();
		}

		_threadPool.clear();
	}

	std::shared_ptr<Task> ThreadManager::PushTask(std::function<void(void)> func, const std::string& name)
	{
		auto task = std::make_shared<Task>(func, name);
		_taskQueue->Push(task);
		return task;
	}

	void ThreadManager::InitializeThreadLocal(const std::string& name)
	{
		SetCurrentThreadName(name);

		static std::atomic<uint32> S_autoIncreaseThreadId = 1;
		LThreadId = S_autoIncreaseThreadId.fetch_add(1);

		EN_LOG_INFO("Initialize Thread Local");
	}

	void ThreadManager::DestroyThreadLocal()
	{
		//	각 Thread별로 TLS 영역에 할당된 SendBuffer 정리 
		// SendBufferArena::ThreadSendBufferClear();
		EN_LOG_INFO("Thread Send Buffer Clear");

		//	MemoryPool에서 각 Thread별로 TLS 영역에 할당된 freeList Memory 정리
		GMemoryPool->ThreadLocalCacheClear();
		EN_LOG_INFO("Thread Local Cache Clear");

		EN_LOG_INFO("Destroy Thread Local");
	}

    void ThreadManager::SetCurrentThreadName(const std::string &name)
  	{
		LThreadName = name;
    	::pthread_setname_np(pthread_self(), name.c_str());
	}

    std::string ThreadManager::GetCurrentThreadName()
    {
		char name[16];
		if (pthread_getname_np(pthread_self(), name, sizeof(name)) == 0) 
		{
			return std::string(name);
		}
		return "Unknown";
    }

    void ThreadManager::WorkerThread()
	{
		InitializeThreadLocal("WortherThread");

		EN_LOG_INFO("ThreadPool Started");

		while (_poolRunning.load(std::memory_order_acquire) == true)
		{
			auto task = _taskQueue->Pop();
			if (task)
				task->Execute();
		}

		DestroyThreadLocal();

		EN_LOG_INFO("ThreadPool finished");
	}
}
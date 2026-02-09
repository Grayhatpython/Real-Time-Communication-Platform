#include "engine/EnginePch.h"
#include "engine/ThreadManager.h"
#include "engine/MemoryPool.h"
#include "engine/Logger.h"

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

	ThreadManager::ThreadManager()
	{
		//	TEMP
		engine::InitializeLThreadState(100, "Main");
		engine::InitializeLThreadLocalCache();
	}

	ThreadManager::~ThreadManager()
	{
		engine::DestroyLThreadLocalCache();
		engine::DestroyLThreadState();
	}

	void ThreadManager::RegisterExitCallback(ThreadRole mask, ExitCallback callback, std::string name)
	{
		std::lock_guard<std::mutex> lock(_lock);
        _callbackEntrys.push_back(CallbackEntry{ mask, std::move(callback), std::move(name) });
	}

    ThreadManager::ThreadHandle ThreadManager::Spawn(std::string name, ThreadRole role, ThreadFunc func)
    {
		const size_t id = _threads.size();

		_threads.push_back(ManagedThread{});
		ManagedThread& managedThread = _threads.back();
		managedThread.name = name;
		managedThread.role = role;

		managedThread.thread = std::jthread([this, id, fn = std::move(func), name, role](std::stop_token st) mutable {
			engine::InitializeLThreadState(id, name);
			engine::InitializeLThreadLocalCache();

			EN_LOG_INFO("Thread Spawn");

			fn(st);

			RunExitCallback(role);

			EN_LOG_INFO("Thread Desawn");
			
			engine::DestroyLThreadLocalCache();
			engine::DestroyLThreadState();
		});

        return ThreadHandle{id};
    }

    void ThreadManager::RequestStop(ThreadHandle handle)
    {
		if (handle.id >= _threads.size()) 
			return;

		_threads[handle.id].thread.request_stop();
    }

    void ThreadManager::Join(ThreadHandle handle)
    {
     	if (handle.id >= _threads.size()) 
			return;

        auto& thread = _threads[handle.id].thread;
        if (thread.joinable()) 
			thread.join();
    }

    void ThreadManager::StopAllAndJoin()
    {
		for (auto& managedThread : _threads)
            managedThread.thread.request_stop();

        for (auto& managedThread : _threads)
		{
            if (managedThread.thread.joinable()) 
				managedThread.thread.join();
		}

        _threads.clear();
    }

    void ThreadManager::RunExitCallback(ThreadRole role)
    {
		std::vector<CallbackEntry> callbackEntrys;
		{
			std::lock_guard<std::mutex> lock(_lock);
			callbackEntrys = _callbackEntrys;
		}

		for(auto& entry : callbackEntrys)
		{
			if(HasRole(entry.mask, role) == false && entry.mask != ThreadRole::None)
				continue;

			entry.callback();
		}
    }
}

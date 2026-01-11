#pragma once

namespace servercore
{
	class MemoryPool;
	class ThreadManager;
	class GlobalContext
	{
	public:
		GlobalContext(const GlobalContext&) = delete;
		GlobalContext& operator=(const GlobalContext&) = delete;
	
	private:
		GlobalContext() = default;
		~GlobalContext();

	public:
		static GlobalContext& GetInstance()
		{
			static GlobalContext instance;
			return instance;
		}
	
		bool Initialize();
		void Clear();

	public:
		MemoryPool* 	GetMemoryPool() { return _memoryPool;}
		ThreadManager* 	GetThreadPool() { return _threadManager;}

	private:
		MemoryPool* 		_memoryPool = nullptr;
		ThreadManager* 	_threadManager = nullptr;
	};

#define GMemoryPool		GlobalContext::GetInstance().GetMemoryPool()
#define GThreadManager	GlobalContext::GetInstance().GetThreadPool()
}


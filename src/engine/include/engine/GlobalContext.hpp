#pragma once

namespace engine
{
	class MemoryPool;
	class ThreadManager;
	class GlobalContext
	{
	public:
		GlobalContext() = default;
		~GlobalContext();
		
		GlobalContext(const GlobalContext&) = delete;
		GlobalContext& operator=(const GlobalContext&) = delete;
	
	public:
		static GlobalContext& GetInstance()
		{
			static GlobalContext instance;
			return instance;
		}
	
		bool Initialize();
		void Clear();

	public:
		MemoryPool* 		GetMemoryPool() { return _memoryPool;}
		ThreadManager* 		GetThreadManager() { return _threadManager;}

	private:
		MemoryPool* 		_memoryPool = nullptr;
		ThreadManager* 		_threadManager = nullptr;
	};
}



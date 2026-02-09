#pragma once

namespace engine
{
	class Time;
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
		Time* 				GetTIme() { return _time;}
		MemoryPool* 		GetMemoryPool() { return _memoryPool;}
		ThreadManager* 		GetThreadManager() { return _threadManager;}

	private:
		Time*				_time = nullptr;
		MemoryPool* 		_memoryPool = nullptr;
		ThreadManager* 		_threadManager = nullptr;
	};
}



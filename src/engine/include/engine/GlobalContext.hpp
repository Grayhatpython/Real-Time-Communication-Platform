#pragma once

namespace engine
{
	class MemoryPool;
	class ThreadManager;
	class SessionManager;
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
		ThreadManager* 		GetThreadPool() { return _threadManager;}
		// SessionManager*		GetSessionManager() { return _sessionManager; }


	private:
		MemoryPool* 		_memoryPool = nullptr;
		ThreadManager* 		_threadManager = nullptr;
		// SessionManager* 	_sessionManager= nullptr;
	};
}

#define GMemoryPool		engine::GlobalContext::GetInstance().GetMemoryPool()
#define GThreadManager	engine::GlobalContext::GetInstance().GetThreadPool()
// #define GSessionManager	engine::GlobalContext::GetInstance().GetSessionManager()


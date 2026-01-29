#include "Pch.hpp"
#include "GlobalContext.hpp"
#include "MemoryPool.hpp"
#include "ThreadManager.hpp"
#include "SendBufferPool.hpp"
#include "SessionManager.hpp"

namespace servercore
{
	GlobalContext::~GlobalContext()
	{
		Clear();
	}

	bool GlobalContext::Initialize()
	{
		_memoryPool = new MemoryPool();
		_threadManager = new ThreadManager();
		_sessionManager = new SessionManager();

		if(_memoryPool == nullptr || _threadManager == nullptr || _sessionManager == nullptr )
			return false;

		Logger::Initialize("Core Logger");

		return true;
	}

	void GlobalContext::Clear()
	{
		//	TODO : Clear
		if(_sessionManager)
		{
			delete _sessionManager;
			_sessionManager = nullptr;
		}

		if (_threadManager)
		{
			delete _threadManager;
			_threadManager = nullptr;
		}

		if (_memoryPool)
		{
			delete _memoryPool;
			_memoryPool = nullptr;
		}
	}
}

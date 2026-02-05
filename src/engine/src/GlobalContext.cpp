#include "engine/GlobalContext.hpp"
#include "engine/MemoryPool.hpp"
#include "engine/ThreadManager.hpp"
#include "engine/Logger.hpp"

namespace engine
{
	GlobalContext::~GlobalContext()
	{
		Clear();
	}

	bool GlobalContext::Initialize()
	{
		Logger::Initialize("Core Logger");

		_memoryPool = new MemoryPool();
		_threadManager = new ThreadManager();
		// _sessionManager = new SessionManager();

		if(_memoryPool == nullptr || _threadManager == nullptr )//_sessionManager == nullptr )
			return false;

		return true;
	}

	void GlobalContext::Clear()
	{
		//	TODO : Clear
		// if(_sessionManager)
		// {
			// delete _sessionManager;
			// _sessionManager = nullptr;
		// }

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

#include "engine/EnginePch.h"
#include "engine/GlobalContext.h"
#include "engine/MemoryPool.h"
#include "engine/ThreadManager.h"
#include "engine/Logger.h"

namespace engine
{
	GlobalContext::~GlobalContext()
	{
	
	}

	bool GlobalContext::Initialize()
	{
		Logger::Initialize("Core Logger");

		_memoryPool = new MemoryPool();
		_threadManager = new ThreadManager();

		if(_memoryPool == nullptr || _threadManager == nullptr )
			return false;

		return true;
	}

	void GlobalContext::Clear()
	{
		if (_memoryPool)
		{
			delete _memoryPool;
			_memoryPool = nullptr;
		}

		if (_threadManager)
		{
			delete _threadManager;
			_threadManager = nullptr;
		}

        //  async_logger 처리 및 스레드 작업 마무리
        engine::Logger::Shutdown();
	}
}

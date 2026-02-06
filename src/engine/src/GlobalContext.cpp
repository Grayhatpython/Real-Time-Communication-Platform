#include "engine/EnginePch.hpp"
#include "engine/GlobalContext.hpp"
#include "engine/MemoryPool.hpp"
#include "engine/ThreadManager.hpp"
#include "engine/Logger.hpp"

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

        //  async_logger 처리 및 스레드 작업 마무리
        engine::Logger::Shutdown();
	}
}

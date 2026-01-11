#include "Pch.hpp"
#include "GlobalContext.hpp"
#include "MemoryPool.hpp"
#include "ThreadManager.hpp"
#include "SendBufferPool.hpp"

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

		if(_memoryPool == nullptr || _threadManager == nullptr)
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
	}
}

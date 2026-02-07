#pragma once
#include "CommonType.hpp"
#include "MemoryPool.hpp"

namespace engine 
{
	// threadLocal "공통"
	struct ThreadState
	{
		uint64 threadId = 0;
		std::string threadName;
	};

	inline thread_local ThreadState* LthreadState = nullptr;

	inline void InitializeLThreadState(uint64 tid, std::string name)
	{
		if (LthreadState == nullptr)
			LthreadState = new ThreadState{ tid , name };
	}

	inline void DestroyLThreadState() noexcept
	{
		if(LthreadState)
		{
			delete LthreadState;
			LthreadState = nullptr;
		}
	}

	//	MemoryPool -> threadLocalCache
	inline thread_local ThreadLocalCache* LThreadLocalCache = nullptr;

	inline void InitializeLThreadLocalCache()
	{
		if (LThreadLocalCache == nullptr)
			LThreadLocalCache = new ThreadLocalCache();
	}

	inline void DestroyLThreadLocalCache() noexcept
	{
		if(LThreadLocalCache)
		{
			delete LThreadLocalCache;
			LThreadLocalCache = nullptr;
		}
	}
} 

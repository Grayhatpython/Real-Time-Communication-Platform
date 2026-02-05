#include "engine/Lock.hpp"

namespace engine
{
	Lock::Lock()
	{
		::pthread_mutex_init(&_mutex, nullptr);
	}

	Lock::~Lock()
	{
		::pthread_mutex_destroy(&_mutex);
	}

	void Lock::WriteLock()
	{
		if (_ownerThreadId.load(std::memory_order_relaxed) == LThreadId)
        {
            _recursion.fetch_add(1, std::memory_order_relaxed);
            return;
        }

		uint32 expected = EMPTY_OWNER_THREAD_ID;

		for (int i = 0; i < MAX_PAUSE_SPIN_COUNT; ++i)
        {
        	expected = EMPTY_OWNER_THREAD_ID;
            if (_ownerThreadId.compare_exchange_weak( expected, LThreadId, std::memory_order_acquire, std::memory_order_relaxed))
            {
                _recursion.store(1, std::memory_order_relaxed);
                return;
            }

			//	유저 모드
            PAUSE();
        }

		pthread_mutex_lock(&_mutex);
            
		EN_LOG_DEBUG("{} Thread -> pthread_mutex_lock", LThreadId);

		while (true)
        {
            if (_ownerThreadId.compare_exchange_strong( expected, LThreadId, std::memory_order_acquire, std::memory_order_relaxed))
            {
                _recursion.store(1, std::memory_order_relaxed);
                ::pthread_mutex_unlock(&_mutex);
                return;
            }

			//	스케줄러 레벨 양보
            ::sched_yield();
        }
	}

	void Lock::WriteUnLock()
	{
        if (_ownerThreadId.load(std::memory_order_relaxed) != LThreadId)
        {
            EN_LOG_ERROR("WriteUnlock by non-owner");
            return;
        }

        // 재진입 카운트 감소
        uint32_t prev = _recursion.fetch_sub(1, std::memory_order_relaxed);

        if (prev == 1)
        {
            // 마지막 unlock
            _ownerThreadId.store(0, std::memory_order_release);
        }
	}
}
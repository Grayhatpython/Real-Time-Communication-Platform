#include "Pch.hpp"
#include "Lock.hpp"

namespace servercore
{
	Lock::Lock()
	{
		pthread_rwlock_init(&_lock, nullptr);

	}

	Lock::~Lock()
	{
		pthread_rwlock_destroy(&_lock);
	}

	void Lock::WriteLock()
	{
		auto ownerThreadId = _ownerThreadId.load();

		if (ownerThreadId == LThreadId)
		{
			_nestedCount.fetch_add(1);
			return;
		}

		while (true)
		{
			uint32 expected = EMPTY_OWNER_THREAD_ID;
			if (_ownerThreadId.compare_exchange_strong(expected, LThreadId))
			{
				pthread_rwlock_wrlock(&_lock);
				_nestedCount.fetch_add(1);
				return;
			}

			for (uint32 i = 0; i < MAX_PAUSE_SPIN_COUNT; i++)
				PAUSE();
		}
	}

	void Lock::WriteUnLock()
	{
		auto ownerThreadId = _ownerThreadId.load();

		if (ownerThreadId == LThreadId)
		{
			auto prevCount = _nestedCount.fetch_sub(1);

			if (prevCount == 1)
			{
				pthread_rwlock_unlock(&_lock);
				_ownerThreadId.store(EMPTY_OWNER_THREAD_ID);
			}
		}
	}
}
#pragma once		

#if defined(__x86_64__) || defined(__i386__)
  #include <immintrin.h>
  inline void PAUSE() { _mm_pause(); }
#else
  inline void PAUSE() {  }
#endif

namespace servercore
{
	class Lock
	{
		enum
		{
			EMPTY_OWNER_THREAD_ID = 0,
			MAX_PAUSE_SPIN_COUNT = 1024,
		};

	public:
		Lock();
		~Lock();

	public:
		void WriteLock();
		void WriteUnLock();

	private:
	    // owner thread id (0=free)
		alignas(CACHE_LINE_SIZE) std::atomic<uint32> _ownerThreadId{EMPTY_OWNER_THREAD_ID};

		// 재진입 횟수 (owner만 수정)
		std::atomic<uint32> _recursion{0};

		// 경쟁 완화용 mutex
		pthread_mutex_t _mutex{};
	};

	class WriteLockGuard
	{
	public:
		WriteLockGuard(Lock& lock)
			: _lock(lock)
		{
			_lock.WriteLock();
		}
		~WriteLockGuard()
		{
			_lock.WriteUnLock();
		}

	private:
		Lock& _lock;
	};
}
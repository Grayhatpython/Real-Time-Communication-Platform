#include "engine/EnginePch.hpp"
#include "engine/ThreadLocal.hpp"

namespace engine
{
	thread_local uint32 LThreadId = 0;
	thread_local std::string LThreadName;
	thread_local std::function<void(void)> LDestroyTLSCallback;
}
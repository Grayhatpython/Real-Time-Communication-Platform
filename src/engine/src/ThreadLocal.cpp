#include "engine/ThreadLocal.hpp"

namespace engine
{
	thread_local uint32 LThreadId = 0;
	thread_local std::string LThreadName;
}
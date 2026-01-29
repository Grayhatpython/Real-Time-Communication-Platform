#include "Pch.hpp"
#include "ThreadLocal.hpp"

namespace servercore
{
	thread_local uint32 LThreadId = 0;
	thread_local std::string LThreadName;
}
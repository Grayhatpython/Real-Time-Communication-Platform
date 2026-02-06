#pragma once

namespace engine
{
	extern thread_local uint32 LThreadId;
	extern thread_local std::string LThreadName;
	extern thread_local std::function<void(void)> LDestroyTLSCallback;
}
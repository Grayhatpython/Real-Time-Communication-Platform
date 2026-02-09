#include "engine/EnginePch.h"
#include "engine/Allocator.h"
#include "engine/MemoryPool.h"

namespace engine
{
	void* Allocator::Allocate(size_t size)
	{
		return GlobalContext::GetInstance().GetMemoryPool()->Allocate(size);
	}

	void Allocator::Deallocate(void* ptr)
	{
		GlobalContext::GetInstance().GetMemoryPool()->Deallocate(ptr);
	}
}
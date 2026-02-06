#include "engine/EnginePch.hpp"
#include "engine/Allocator.hpp"
#include "engine/MemoryPool.hpp"

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
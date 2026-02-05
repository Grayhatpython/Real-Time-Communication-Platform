#include "engine/Allocator.hpp"
#include "engine/MemoryPool.hpp"

namespace engine
{
	void* Allocator::Allocate(size_t size)
	{
		return GMemoryPool->Allocate(size);
	}

	void Allocator::Deallocate(void* ptr)
	{
		GMemoryPool->Deallocate(ptr);
	}
}
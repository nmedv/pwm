#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

	void* Allocate(size_t size);
	void Deallocate(void* ptr);
	void DeallocateArray(void** ptr);
	// inline void Reallocate(void* ptr, size_t newSize);

};

#endif // !MEMORY_MANAGER_H
#include "memory.h"

#include "logger.h"

bool Memory__allocate(
	__in const uint32_t size,
	__in void* const requested_base_address,
	__out MemoryMap* const memory)
{
	void* allocated_ptr = NULL;

	if (memory == NULL)
	{
		DEBUG_LOG(L"Invalid MemoryMap parameter given.");
		return false;
	}
	
	if (requested_base_address == NULL)
	{
		DEBUG_LOG(L"Invalid requested_base_address parameter given.");
		return false;
	}

	memory->size = 0;
	memory->start_pointer = NULL;

	allocated_ptr = VirtualAlloc(
		requested_base_address,
		size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	if (allocated_ptr == NULL)
	{
		DEBUG_LOG(L"Failed to allocate memory with windows error %u", GetLastError());
		return false;
	}

	memory->start_pointer = allocated_ptr;
	memory->size = size;
	return true;
}

bool Memory__free(__in MemoryMap* const memory)
{
	if (memory == NULL)
	{
		DEBUG_LOG(L"Invalid MemoryMap parameter given.");
		return false;
	}

	if (memory->start_pointer == NULL)
	{
		DEBUG_LOG(L"MemoryMap is not initialized");
		return false;
	}

	static const SIZE_T RELEASE_ALL = 0;

	if (!VirtualFree(memory->start_pointer, RELEASE_ALL, MEM_RELEASE))
	{
		DEBUG_LOG(L"VirtualFree() failed with windows error %u", GetLastError());
		return false;
	}

	memory->size = 0;
	memory->start_pointer = NULL;
	return true;
}
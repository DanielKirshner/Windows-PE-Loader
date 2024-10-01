#include "memory.h"

#include "logger.h"

bool Memory__allocate(
	__in const size_t size,
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
		DEBUG_LOG(L"Failed to allocate memory with Windows error %u", GetLastError());
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

	static const size_t RELEASE_ALL = 0;

	if (!VirtualFree(memory->start_pointer, RELEASE_ALL, MEM_RELEASE))
	{
		DEBUG_LOG(L"VirtualFree() failed with Windows error %u", GetLastError());
		return false;
	}

	memory->size = 0;
	memory->start_pointer = NULL;
	return true;
}

bool Memory__set_protection(
	const MemoryMap* const memory,
	const size_t rva_address,
	const size_t size,
	const DWORD protection)
{
	if (memory == NULL)
	{
		DEBUG_LOG(L"Invalid memory parameter given.");
		return false;
	}

	if (memory->start_pointer == NULL)
	{
		DEBUG_LOG(L"MemoryMap given is not initialized");
		return false;
	}

	DWORD __out old_protection = 0;
	
	if (!VirtualProtect(memory->start_pointer + rva_address, size, protection, &old_protection))
	{
		DEBUG_LOG(L"VirtualProtect() failed to set protection with Windows error %u", GetLastError());
		return false;
	}
	return true;
}

bool Memory__copy(
	const MemoryMap* const memory,
	const size_t rva_address,
	const uint8_t* const buffer,
	const size_t buffer_size)
{
	const uint8_t* buffer_ptr = NULL;
	uint8_t* memory_ptr = NULL;

	if (memory == NULL || buffer == NULL)
	{
		DEBUG_LOG(L"Invalid memory or buffer parameter given.");
		return false;
	}

	if (buffer_size > memory->size - rva_address)
	{
		DEBUG_LOG(L"The buffer given is too big.");
		return false;
	}

	buffer_ptr = buffer;
	memory_ptr = memory->start_pointer + rva_address;
	__movsb(memory_ptr, buffer_ptr, buffer_size);
	return true;
}

bool Memory__rva_to_absolute(
	const MemoryMap* const memory,
	const size_t rva_address,
	uint8_t** const actual_address)
{
	if (memory == NULL)
	{
		DEBUG_LOG(L"Invalid memory parameter given.");
		return false;
	}

	if (actual_address == NULL)
	{
		DEBUG_LOG(L"Invalid actual_address parameter given.");
		return false;
	}

	if (memory->start_pointer == NULL)
	{
		DEBUG_LOG(L"Memory not initialized given.");
		return false;
	}

	if (rva_address >= memory->size)
	{
		DEBUG_LOG("The RVA (relative virtual address) is out of range.");
		return false;
	}


	*actual_address = memory->start_pointer + rva_address;
	return true;
}
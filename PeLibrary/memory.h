#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

typedef struct _MemoryMap
{
	uint8_t* start_pointer;
	uint32_t size;
} MemoryMap;

/**
	@brief
		Allocates memory using VirtualAlloc of a given size at a requested base address and updates the provided MemoryMap.
		It reserves and commits the memory block with the given size at the specified `requested_base_address`.
		If the allocation is successful, the allocated memory's pointer and size are stored in the `MemoryMap`.
		In case of invalid parameters or allocation failure, it logs an error and returns `false`.
	@param
		`size` The size of the memory block to allocate, in bytes.
	@param
		`requested_base_address` Pointer to the requested base address for the allocation.
	@param
		`memory` Pointer to the `MemoryMap` that will be updated with the allocated memory's pointer and size.
	@return `true` if memory allocation succeeds, `false` otherwise.
			On failure, appropriate errors are logged.
*/
bool Memory__allocate(
	__in const uint32_t size,
	__in void* const requested_base_address,
	__out MemoryMap* const memory);

/**
	@brief
		Frees memory allocated in the provided MemoryMap.
		Releases the memory block pointed to by `memory->start_pointer` using `VirtualFree`.
		If the `MemoryMap` or `start_pointer` is `NULL`, or if `VirtualFree` fails, the function logs an error and returns `false`.
		On success, it resets the `MemoryMap`'s fields.
	@param
		memory Pointer to a `MemoryMap` containing the memory to free.
	@return
		`true` if the memory is successfully freed, `false` on failure.
*/
bool Memory__free(__in MemoryMap* const memory);
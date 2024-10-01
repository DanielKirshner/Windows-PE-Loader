#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

typedef struct _MemoryMap
{
	uint8_t* start_pointer;
	size_t size;
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
	__in const size_t size,
	__in void* const requested_base_address,
	__out MemoryMap* const memory);

/**
@brief
	Frees memory allocated in the provided MemoryMap.
	Releases the memory block pointed to by `memory->start_pointer` using `VirtualFree`.
	If the `MemoryMap` or `start_pointer` is `NULL`, or if `VirtualFree` fails, the function logs an error and returns `false`.
	On success, it resets the `MemoryMap`'s fields.
@param
	`memory` Pointer to a `MemoryMap` containing the memory to free.
@return
	`true` if the memory is successfully freed, `false` on failure.
*/
bool Memory__free(__in MemoryMap* const memory);


/**
@brief
	Sets memory protection for a specified range within a MemoryMap.
	Applies the given protection flags (read, write, execute) to a range of memory
	starting at the specified relative virtual address (RVA) within the provided MemoryMap.
@param
	`memory` Pointer to a `MemoryMap` structure.
@param
	`rva_address` The relative virtual address where the protection should be applied.
@param
	`size` The size of the memory range to modify.
@param
	`protection` The protection flags.
@return
	True if successful, false otherwise.
 */
bool Memory__set_protection(
	__in const MemoryMap* const memory,
	__in const size_t rva_address,
	__in const size_t size,
	__in const DWORD protection);

/**
@brief
	Copies data from a buffer into memory at a specified RVA.
	Copies the contents of a buffer into a memory location within the MemoryMap,
	starting from the specified relative virtual address.
 @param
	`memory` Pointer to a `MemoryMap` structure.
 @param
	`rva_address` Starting relative virtual address for the data copy.
 @param
	`buffer` Pointer to the data buffer to copy from.
 @param
	`buffer_size` The size of the data buffer.
 @return
	True if the copy was successful, false if the memory or buffer is invalid.
 */
bool Memory__copy(
	__in const MemoryMap* const memory,
	__in const size_t rva_address,
	__in const uint8_t* const buffer,
	__in const size_t buffer_size);

/**
@brief
	Converts an RVA to an absolute memory address.
	This function translates a relative virtual address (RVA) to
	an absolute memory address within the given MemoryMap.
@param
	`memory` Pointer to a `MemoryMap` structure.
@param
	`rva_address` The relative virtual address to convert.
@param
	`actual_address` A pointer to a pointer that will hold the resulting absolute address.
@return
	True if the conversion is successful, false otherwise.
 */
bool Memory__rva_to_absolute(
	__in const MemoryMap* const memory,
	__in const size_t rva_address,
	__out uint8_t** const actual_address);
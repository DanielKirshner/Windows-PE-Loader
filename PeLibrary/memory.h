#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>


typedef struct _MemoryMap
{
	uint8_t* start_pointer;
	uint32_t size;
} MemoryMap;

bool Memory__allocate(
	const uint32_t size,
	void* const requested_base_address,
	MemoryMap* const memory);
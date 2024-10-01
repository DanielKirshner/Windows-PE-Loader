#pragma once

#include <stdint.h>
#include <Windows.h>

#include "memory.h"

typedef struct _LibraryModule
{
	IMAGE_DOS_HEADER* dos_header;
	IMAGE_NT_HEADERS* nt_headers;
	MemoryMap memory;
} LibraryModule;

HMODULE __stdcall Library__load_library(
	__in const uint8_t* const pe_buffer,
	__in const size_t pe_buffer_size);
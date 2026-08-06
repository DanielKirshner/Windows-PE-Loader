#pragma once

#include <stdint.h>
#include <Windows.h>

#include "memory.h"
#include "imports.h"

typedef BOOL(WINAPI* DllMainFunction)(HINSTANCE, const DWORD, LPVOID);

typedef struct _LibraryModule
{
	IMAGE_DOS_HEADER* dos_header;
	IMAGE_NT_HEADERS* nt_headers;
	MemoryMap memory;
	Imports imports;
} LibraryModule;

HMODULE __stdcall Library__LoadLibrary(
	__in const uint8_t* const pe_buffer,
	__in const uint32_t pe_buffer_size);

BOOL __stdcall Library__FreeLibrary(
	const HMODULE instance);
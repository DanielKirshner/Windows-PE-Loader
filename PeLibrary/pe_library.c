#include "pe_library.h"

#include "utils.h"
#include "logger.h"

#include <stdbool.h>

static bool Library__validate_pe(
	__in const uint8_t* const pe_buffer,
	__in const size_t pe_buffer_size,
	__out LibraryModule* const module)
{
	if (sizeof(IMAGE_DOS_HEADER) > pe_buffer_size)
	{
		DEBUG_LOG(L"DOS header is too big for the pe buffer");
		return false;
	}

	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)(pe_buffer);

	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		DEBUG_LOG(L"Invalid magic in DOS header");
		return false;
	}
	
	if (dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS) > pe_buffer_size)
	{
		DEBUG_LOG(L"PE buffer is too small for NT headers");
		return false;
	}

	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(pe_buffer + dos_header->e_lfanew);

	if (nt_header->Signature != IMAGE_NT_SIGNATURE)
	{
		DEBUG_LOG(L"Invalid magic in NT headers");
		return false;
	}

	if (!HAS_FLAG(nt_header->FileHeader.Characteristics, IMAGE_FILE_32BIT_MACHINE))
	{
		DEBUG_LOG(L"PE is not 32bit");
		return false;
	}

	if (!HAS_FLAG(nt_header->FileHeader.Characteristics, IMAGE_FILE_DLL))
	{
		DEBUG_LOG(L"PE given is not a DLL");
		return false;
	}

	module->dos_header = dos_header;
	module->nt_headers = nt_header;
	return true;
}

static bool Library__allocate_memory(LibraryModule* const module)
{
	uint8_t* const requested_begin_address = (uint8_t* const)module->nt_headers->OptionalHeader.ImageBase;

	if (!Memory__allocate(
		module->nt_headers->OptionalHeader.SizeOfImage,
		requested_begin_address,
		&module->memory))
	{
		DEBUG_LOG(L"Failed to allocate memory");
		return false;
	}
	return true;
}

static bool Library__copy_section(
	const uint8_t* const pe_buffer,
	LibraryModule* const module,
	const uint32_t current_raw_offset)
{
	const IMAGE_SECTION_HEADER* const section_header = (const IMAGE_SECTION_HEADER* const)(pe_buffer + current_raw_offset);

	if (!Memory__copy(
		&module->memory,
		section_header->VirtualAddress,
		pe_buffer + section_header->PointerToRawData,
		min(section_header->SizeOfRawData, section_header->Misc.VirtualSize)))
	{
		DEBUG_LOG(L"Failed to copy section data");
		return false;
	}
	return true;
}

HMODULE __stdcall Library__load_library(
	__in const uint8_t* const pe_buffer,
	__in const size_t pe_buffer_size)
{
	LibraryModule module = { 0 };

	if (!Library__validate_pe(pe_buffer, pe_buffer_size, &module))
	{
		DEBUG_LOG(L"Validation of PE failed!");
		return NULL;
	}
	return (HMODULE)(module.memory.start_pointer);
}
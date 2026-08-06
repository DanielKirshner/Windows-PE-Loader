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
	
	if (dos_header->e_lfanew < 0 ||
		(size_t)dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS) > pe_buffer_size)
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
	const uint32_t pe_buffer_size,
	LibraryModule* const module,
	const uint32_t current_raw_offset)
{
	const IMAGE_SECTION_HEADER* const section_header = (const IMAGE_SECTION_HEADER* const)(pe_buffer + current_raw_offset);

	const uint32_t copy_size = min(section_header->SizeOfRawData, section_header->Misc.VirtualSize);

	if (section_header->PointerToRawData > pe_buffer_size ||
		copy_size > pe_buffer_size - section_header->PointerToRawData)
	{
		DEBUG_LOG(L"Section raw data exceeds PE buffer bounds");
		return false;
	}

	if (!Memory__copy(
		&module->memory,
		section_header->VirtualAddress,
		pe_buffer + section_header->PointerToRawData,
		copy_size))
	{
		DEBUG_LOG(L"Failed to copy section data");
		return false;
	}
	return true;
}

static bool Library__update_headers_after_copy(LibraryModule* const module)
{
	module->dos_header = (IMAGE_DOS_HEADER*)module->memory.start_pointer;
	module->nt_headers = (IMAGE_NT_HEADERS*)(module->memory.start_pointer + module->dos_header->e_lfanew);
	return true;
}

static bool Library__copy_to_memory(
	const uint8_t* const pe_buffer,
	const uint32_t pe_buffer_size,
	LibraryModule* const module)
{
	static const size_t MEMORY_BEGIN_ADDRESS = 0;

	if (!Memory__copy(
		&module->memory,
		MEMORY_BEGIN_ADDRESS,
		pe_buffer,
		module->nt_headers->OptionalHeader.SizeOfHeaders))
	{
		DEBUG_LOG(L"Failed to copy headers");
		return false;
	}

	Library__update_headers_after_copy(module);

	uint32_t current_raw_offset = module->dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS);

	for (uint32_t i = 0; i < module->nt_headers->FileHeader.NumberOfSections; i++)
	{
		if (current_raw_offset + sizeof(IMAGE_SECTION_HEADER) >= pe_buffer_size)
		{
			DEBUG_LOG(L"PE buffer is too small for section headers");
			return false;
		}
		
		if (!Library__copy_section(pe_buffer, pe_buffer_size, module, current_raw_offset))
		{
			DEBUG_LOG(L"Failed to copy section");
			return false;
		}

		current_raw_offset += sizeof(IMAGE_SECTION_HEADER);
	}
	return true;
}

static bool Library__set_memory_protections(LibraryModule* const module)
{
	static const uint32_t MEMORY_BEGIN_ADDRESS = 0;
	if (!Memory__set_protection(&module->memory, MEMORY_BEGIN_ADDRESS, module->memory.size, PAGE_READONLY))
	{
		DEBUG_LOG(L"Failed to set initial readonly protection");
		return false;
	}

	uint8_t* current_section_ptr = NULL;
	RVA_TO_ABSOLUTE(
		&module->memory,
		module->dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS),
		current_section_ptr);

	for (uint32_t i = 0; i < module->nt_headers->FileHeader.NumberOfSections; i++)
	{
		const IMAGE_SECTION_HEADER* const section_header = (const IMAGE_SECTION_HEADER* const)current_section_ptr;
		size_t protection = 0;
		
		if (section_header != NULL &&
			!Utils__section_characteristics_to_protection_flag(
				section_header->Characteristics,
				&protection)
			)
		{
			DEBUG_LOG(L"Failed to identify section characteristics");
			return false;
		}

		if (section_header != NULL &&
			!Memory__set_protection(
				&module->memory,
				section_header->VirtualAddress,
				section_header->Misc.VirtualSize,
				protection)
			)
		{
			return false;
		}

		current_section_ptr += sizeof(IMAGE_SECTION_HEADER);
	}

	return true;
}

static bool Library__call_main(LibraryModule* const module, const DWORD reason)
{
	uint8_t* func_address = NULL;

	RVA_TO_ABSOLUTE(
		&module->memory,
		module->nt_headers->OptionalHeader.AddressOfEntryPoint,
		func_address);

	const DllMainFunction func = (DllMainFunction)func_address;

	const bool dll_func_return = func((HMODULE)module->memory.start_pointer, reason, NULL);

	if (reason == DLL_PROCESS_ATTACH)
	{
		return dll_func_return;
	}

	return true;
}

static bool Library__handle_import_table(LibraryModule* const module, const bool do_load_libraries)
{
	const IMAGE_DATA_DIRECTORY import_table_directory =
		module->nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!Imports__set_imports(
			import_table_directory,
			&module->memory,
			&module->imports,
			do_load_libraries)
		)
	{
		return false;
	}
	return true;
}


HMODULE __stdcall Library__load_library(const uint8_t* const pe_buffer, const uint32_t pe_buffer_size)
{
	LibraryModule module = { 0 };

	if (!Library__validate_pe(pe_buffer, pe_buffer_size, &module))
	{
		DEBUG_LOG(L"Validation of PE failed");
		goto cleanup;
	}

	if (!Library__allocate_memory(&module))
	{
		DEBUG_LOG(L"Failed allocating memory");
		goto cleanup_memory;
	}

	if (!Library__copy_to_memory(pe_buffer, pe_buffer_size, &module))
	{
		DEBUG_LOG(L"Failed to copy memory");
		goto cleanup_memory;
	}

	static const bool LOAD_LIBRARIES = true;
	if (!Library__handle_import_table(&module, LOAD_LIBRARIES))
	{
		DEBUG_LOG(L"Fix import table failed");
		goto cleanup_memory;
	}

	if (!Library__set_memory_protections(&module))
	{
		DEBUG_LOG(L"Set memory protections failed");
		goto cleanup_all;
	}

	if (!Library__call_main(&module, DLL_PROCESS_ATTACH))
	{
		DEBUG_LOG(L"Run main failed");
		goto cleanup_all;
	}

	return (HMODULE)module.memory.start_pointer;

cleanup_all:
	Imports__clean_imports(&module.memory, &module.imports);

cleanup_memory:
	Memory__free(&module.memory);

cleanup:
	return NULL;
}

BOOL __stdcall Library__FreeLibrary(const HMODULE instance)
{
	if (instance == NULL)
	{
		DEBUG_LOG(L"Invalid NULL instance passed to FreeLibrary");
		return false;
	}

	LibraryModule module = { 0 };
	module.memory.start_pointer = (void*)instance;

	const IMAGE_DOS_HEADER* dos_header = (const IMAGE_DOS_HEADER*)instance;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		DEBUG_LOG(L"Instance does not point to a valid PE (bad DOS signature)");
		return false;
	}

	const IMAGE_NT_HEADERS* nt_headers = (const IMAGE_NT_HEADERS*)((uint8_t*)instance + dos_header->e_lfanew);
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		DEBUG_LOG(L"Instance does not point to a valid PE (bad NT signature)");
		return false;
	}

	module.memory.size = nt_headers->OptionalHeader.SizeOfImage;

	if (!Library__update_headers_after_copy(&module))
	{
		DEBUG_LOG(L"Failed to update headers after copy");
		return false;
	}

	BOOL return_value = true;

	static const bool LIBRARIES_ALREADY_LOADED = false;
	if (!Library__handle_import_table(&module, LIBRARIES_ALREADY_LOADED))
	{
		return_value = false;
		goto cleanup;
	}

	Library__call_main(&module, DLL_PROCESS_DETACH);

	if (!Imports__clean_imports(&module.memory, &module.imports))
	{
		DEBUG_LOG(L"Clean imports failed");
		return_value = false;
	}

cleanup:
	if (!Memory__free(&module.memory))
	{
		DEBUG_LOG(L"Free memory failed");
		return_value = false;
	}

	return return_value;
}

#include "imports.h"

#include "logger.h"
#include "dll.h"
#include "utils.h"

#include <stdint.h>

static bool Imports__replace_function_address(
	const MemoryMap* const memory,
	Dll* const library,
	IMAGE_THUNK_DATA* thunk_data)
{
	FARPROC func_address = NULL;

	if (IMAGE_SNAP_BY_ORDINAL(thunk_data->u1.AddressOfData))
	{
		const uint32_t ordinal = IMAGE_ORDINAL32(thunk_data->u1.AddressOfData);

		if (!Dll__get_function_by_ordinal(library, ordinal, &func_address))
		{
			return false;
		}
	}
	else
	{
		uint8_t* import_by_name_p = NULL;
		RVA_TO_ABSOLUTE(memory, thunk_data->u1.AddressOfData, import_by_name_p);

		const IMAGE_IMPORT_BY_NAME* const import_by_name = (const IMAGE_IMPORT_BY_NAME* const)import_by_name_p;

		const char* const function_name = (const char* const)import_by_name->Name;

		if (!Dll__get_function_by_name(library, function_name, &func_address))
		{
			return false;
		}
	}
	thunk_data->u1.Function = (DWORD)func_address;
	return true;
}

static bool Imports__set_library(
	const MemoryMap* const memory,
	const IMAGE_IMPORT_DESCRIPTOR* const import_descriptor,
	Dll* const library,
	const bool do_load_libraries)
{
	uint8_t* library_name_p = NULL;
	RVA_TO_ABSOLUTE(memory, import_descriptor->Name, library_name_p);

	if (!do_load_libraries)
	{
		if (!Dll__from_handle((char*)library_name_p, library))
		{
			goto cleanup;
		}

		return true;
	}

	if (!Dll__load((char*)library_name_p, library))
	{
		goto cleanup;
	}

	uint8_t* current_thunk_pointer = NULL;
	RVA_TO_ABSOLUTE(memory, import_descriptor->FirstThunk, current_thunk_pointer);

	for (;; current_thunk_pointer += sizeof(IMAGE_THUNK_DATA))
	{
		IMAGE_THUNK_DATA* const thunk_data = (IMAGE_THUNK_DATA* const)current_thunk_pointer;

		static const DWORD END_OF_THUNK = 0;
		if (thunk_data->u1.AddressOfData == END_OF_THUNK)
		{
			break;
		}
		if (!Imports__replace_function_address(memory, library, thunk_data))
		{
			goto cleanup;
		}
	}
	return true;

cleanup:
	Dll__free(library);
	return false;
}

bool Imports__set_imports(
	const IMAGE_DATA_DIRECTORY import_table_directory,
	const MemoryMap* const memory,
	Imports* const imports,
	const bool do_load_libraries)
{
	if (imports == NULL)
	{
		DEBUG_LOG(L"Imports parameter is NULL!");
		return false;
	}

	if (memory == NULL)
	{
		DEBUG_LOG(L"Memory parameter is NULL!");
		return false;
	}
	
	imports->num_libraries = 0;

	uint8_t* current_import_directory_pointer = NULL;
	RVA_TO_ABSOLUTE(memory, import_table_directory.VirtualAddress, current_import_directory_pointer);

	imports->import_directory_beginning_pointer = current_import_directory_pointer;

	static const uint32_t END_OF_DATA = 0;

	for (IMAGE_IMPORT_DESCRIPTOR* import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*)current_import_directory_pointer;
		import_descriptor->Characteristics != END_OF_DATA;
		current_import_directory_pointer += sizeof(IMAGE_IMPORT_DESCRIPTOR),
		import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*)current_import_directory_pointer)
	{
		Dll current_dll = { 0 };

		if (!Imports__set_library(
			memory,
			import_descriptor,
			&current_dll,
			do_load_libraries))
		{
			DEBUG_LOG(L"Failed to add imports for library");
			goto cleanup;
		}

		imports->num_libraries++;
	}
	return true;

cleanup:
	Imports__clean_imports(memory, imports);
	return false;
}

bool Imports__clean_imports(const MemoryMap* const memory, Imports* const imports)
{
	if (imports == NULL)
	{
		DEBUG_LOG(L"Bad imports parameter");
		return false;
	}
	
	if (imports->import_directory_beginning_pointer == NULL)
	{
		DEBUG_LOG(L"imports param contains invalid pointer");
		return false;
	}

	bool return_value = true;

	const uint8_t* current_import_directory_pointer = imports->import_directory_beginning_pointer;
	static const uint32_t END_OF_DATA = 0;

	for (IMAGE_IMPORT_DESCRIPTOR* import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*)current_import_directory_pointer;
		import_descriptor->Characteristics != END_OF_DATA;
		current_import_directory_pointer += sizeof(IMAGE_IMPORT_DESCRIPTOR),
		import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*)current_import_directory_pointer)
	{
		uint8_t* library_name_p = NULL;
		RVA_TO_ABSOLUTE(memory, import_descriptor->Name, library_name_p);

		Dll current_lib = { 0 };

		if (!Dll__from_handle((char*)library_name_p, &current_lib))
		{
			DEBUG_LOG(L"Failed to get Dll from handle");
			return_value = false;
			continue;
		}

		if (!Dll__free(&current_lib))
		{
			DEBUG_LOG(L"Failed to free Dll");
			return_value = false;
		}
	}
	return return_value;
}
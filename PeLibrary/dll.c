#include "dll.h"

#include "logger.h"

bool Dll__load(const char* const path, Dll* const dll)
{
	if (path == NULL)
	{
		DEBUG_LOG(L"Bad path parameter");
		return false;
	}

	dll->handle = NULL;
	const HMODULE module_handle = LoadLibraryA(path);
	
	static const HMODULE LOAD_LIBRARY_FAILED = NULL;
	if (module_handle == LOAD_LIBRARY_FAILED)
	{
		DEBUG_LOG(L"Failed to load library with windows error %u", GetLastError());
		return false;
	}

	dll->handle = module_handle;
	return true;
}

bool Dll__free(Dll* const dll)
{
	if (dll == NULL)
	{
		DEBUG_LOG(L"Bad dll parameter given");
		return false;
	}
	
	if (dll->handle == NULL)
	{
		DEBUG_LOG(L"Dll is not initialized");
		return false;
	}

	if (!FreeLibrary(dll->handle))
	{
		return false;
	}

	dll->handle = NULL;
	return true;
}

bool Dll__from_handle(const char* const path, Dll* const dll)
{
	if (path == NULL || dll == NULL)
	{
		DEBUG_LOG(L"Invalid path or dll parameter given!");
		return false;
	}
	
	dll->handle = NULL;
	const HMODULE module_handle = GetModuleHandleA(path);
	
	if (module_handle == NULL)
	{
		DEBUG_LOG(L"Failed to get library from handle with Windows error %u", GetLastError());
		return false;
	}

	dll->handle = module_handle;
	return true;
}

bool Dll__get_function_by_name(const Dll* const dll, const char* const func_name, FARPROC* const function)
{
	if (dll == NULL || func_name == NULL || function == NULL)
	{
		DEBUG_LOG(L"One or more parameters given are invalid!");
		return false;
	}

	if (dll->handle == NULL)
	{
		DEBUG_LOG(L"Dll given is not initialized!");
		return false;
	}

	*function = GetProcAddress(dll->handle, func_name);

	if (*function == NULL)
	{
		DEBUG_LOG(L"Failed to get address of function %s", func_name);
		return false;
	}
	return true;
}

bool Dll__get_function_by_ordinal(const Dll* const dll, const uint32_t ordinal, FARPROC* const function)
{
	if (dll == NULL || function == NULL)
	{
		DEBUG_LOG(L"One or more parameters given are invalid!");
		return false;
	}

	if (dll->handle == NULL)
	{
		DEBUG_LOG(L"Dll given is not initialized!");
		return false;
	}
	*function = GetProcAddress(dll->handle, MAKEINTRESOURCEA(ordinal));

	if (*function == NULL)
	{
		DEBUG_LOG(L"Failed to get address with ordinal %u", ordinal);
		return false;
	}
	return true;
}
#include "dll.h"

#include "logger.h"

bool Dll__load(
	__in const char* const path,
	__out Dll* const dll)
{
	if (path == NULL || dll == NULL)
	{
		DEBUG_LOG(L"Bad path or dll parameter");
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

bool Dll__free(__in Dll* const dll)
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

bool Dll__from_handle(
	__in const char* const path,
	__out Dll* const dll)
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

bool Dll__get_function_by_name(
	__in const Dll* const dll,
	__in const char* const func_name,
	__out FARPROC* const function)
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
		DEBUG_LOG(L"Failed to get address of function %hs", func_name);
		return false;
	}
	return true;
}

bool Dll__get_function_by_ordinal(
	__in const Dll* const dll,
	__in const size_t ordinal,
	__out FARPROC* const function)
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
		DEBUG_LOG(L"Failed to get address with ordinal %zu", ordinal);
		return false;
	}
	return true;
}
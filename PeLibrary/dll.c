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
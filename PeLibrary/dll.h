#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

typedef struct _Dll
{
	HMODULE handle;
} Dll;

/**
 * Loads a DLL from the specified file path.
 *
 * @param path The file path of the DLL to load.
 * @param dll Pointer to a Dll structure to store the loaded handle.
 * @return true if the DLL was successfully loaded, false otherwise.
 */
bool Dll__load(
	__in const char* const path,
	__out Dll* const dll);

/**
 * Frees a previously loaded DLL.
 *
 * @param dll Pointer to the Dll structure containing the DLL handle.
 * @return true if the DLL was successfully freed, false otherwise.
 */
bool Dll__free(__in Dll* const dll);

/**
 * Retrieves a handle to a previously loaded DLL without loading it.
 *
 * @param path The name of the loaded DLL.
 * @param dll Pointer to a Dll structure to store the retrieved handle.
 * @return true if the handle was successfully retrieved, false otherwise.
 */
bool Dll__from_handle(
	__in const char* const path,
	__out Dll* const dll);

/**
 * Retrieves the address of an exported function from a loaded DLL by name.
 *
 * @param dll Pointer to the Dll structure containing the DLL handle.
 * @param func_name The name of the function to retrieve.
 * @param function Pointer to store the retrieved function address.
 * @return true if the function was successfully retrieved, false otherwise.
 */
bool Dll__get_function_by_name(
	__in const Dll* const dll,
	__in const char* const func_name,
	__out FARPROC* const function);

/**
 * Retrieves the address of an exported function from a loaded DLL by ordinal.
 *
 * @param dll Pointer to the Dll structure containing the DLL handle.
 * @param ordinal The ordinal number of the function to retrieve.
 * @param function Pointer to store the retrieved function address.
 * @return true if the function was successfully retrieved, false otherwise.
 */
bool Dll__get_function_by_ordinal(
	__in const Dll* const dll,
	__in const size_t ordinal,
	__out FARPROC* const function);
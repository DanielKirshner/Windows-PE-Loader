#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

typedef struct _Dll
{
	HMODULE handle;
} Dll;

bool Dll__load(const char* const path, Dll* const dll);

bool Dll__free(Dll* const dll);

bool Dll__from_handle(const char* const path, Dll* const dll);

bool Dll__get_function_by_name(
	const Dll* const dll,
	const char* const func_name,
	FARPROC* const function);

bool Dll__get_function_by_ordinal(
	const Dll* const dll,
	const uint32_t ordinal,
	FARPROC* const function);
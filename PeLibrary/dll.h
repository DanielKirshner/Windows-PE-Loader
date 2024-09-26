#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <Windows.h>

typedef struct _Dll
{
	HMODULE handle;
} Dll;

bool Dll__load(
	const char* const path,
	Dll* const dll);

bool Dll__free(Dll* const dll);
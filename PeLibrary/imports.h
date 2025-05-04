#pragma once

#include "memory.h"

#include <stdbool.h>
#include <Windows.h>

typedef struct
{
	uint32_t num_libraries;
	uint8_t* import_directory_beginning_pointer;
} Imports;

bool Imports__set_imports(
	const IMAGE_DATA_DIRECTORY import_table_directory,
	const MemoryMap* const memory,
	Imports* const imports,
	const bool do_load_libraries);

bool Imports__clean_imports(
	const MemoryMap* const memory,
	Imports* const imports);
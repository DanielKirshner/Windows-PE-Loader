#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>

#include "pe_library.h"

static BOOL read_file(const wchar_t* path, uint8_t** out_buffer, DWORD* out_size)
{
	HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Failed to open file: %s (error: %lu)\n", path, GetLastError());
		return FALSE;
	}

	DWORD file_size = GetFileSize(file, NULL);
	if (file_size == INVALID_FILE_SIZE)
	{
		wprintf(L"Failed to get file size (error: %lu)\n", GetLastError());
		CloseHandle(file);
		return FALSE;
	}

	uint8_t* buffer = (uint8_t*)malloc(file_size);
	if (buffer == NULL)
	{
		wprintf(L"Failed to allocate %lu bytes\n", file_size);
		CloseHandle(file);
		return FALSE;
	}

	DWORD bytes_read = 0;
	if (!ReadFile(file, buffer, file_size, &bytes_read, NULL) || bytes_read != file_size)
	{
		wprintf(L"Failed to read file (error: %lu)\n", GetLastError());
		free(buffer);
		CloseHandle(file);
		return FALSE;
	}

	CloseHandle(file);
	*out_buffer = buffer;
	*out_size = file_size;
	return TRUE;
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2)
	{
		wprintf(L"Usage: %s <path_to_dll>\n", argv[0]);
		return 1;
	}

	uint8_t* pe_buffer = NULL;
	DWORD pe_size = 0;

	if (!read_file(argv[1], &pe_buffer, &pe_size))
	{
		return 1;
	}

	wprintf(L"Loading DLL: %s (%lu bytes)\n", argv[1], pe_size);

	HMODULE module = Library__load_library(pe_buffer, pe_size);
	free(pe_buffer);

	if (module == NULL)
	{
		wprintf(L"Failed to load library\n");
		return 1;
	}

	wprintf(L"Library loaded at 0x%p\n", (void*)module);
	wprintf(L"Press Enter to unload and exit...\n");
	getwchar();

	if (!Library__FreeLibrary(module))
	{
		wprintf(L"Failed to free library\n");
		return 1;
	}

	wprintf(L"Library unloaded successfully\n");
	return 0;
}

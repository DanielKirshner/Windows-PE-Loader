#include "logger.h"

#include "Windows.h"
#include <Shlwapi.h>
#include <stdint.h>

#ifdef _DEBUG
#define MAX_OUTPUT_BUFFER ((size_t)1024)

void Logger__debug_log(const wchar_t* const format, ...)
{
	wchar_t message[MAX_OUTPUT_BUFFER];
	va_list args;
	va_start(args, format);

	const int32_t num_of_wide_chars_written_to_buffer = wvnsprintfW(
		message,
		ARRAYSIZE(message),
		format,
		args);

	if (num_of_wide_chars_written_to_buffer > 0)
	{
		OutputDebugStringW(message);
	}

	va_end(args);
}
#endif
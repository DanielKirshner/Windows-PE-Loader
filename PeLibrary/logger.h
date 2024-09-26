#pragma once

#include <wchar.h>

#ifdef _DEBUG
	void Logger__debug_log(const wchar_t* const format, ...);
	#define DEBUG_LOG(msg, ...) Logger__debug_log(__FUNCTIONW__ L"@ %d: " msg L"\n", __LINE__, ##__VA_ARGS__);

#else
	#define DEBUG_LOG(...)

#endif
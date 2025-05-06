# Windows-PE-Loader
![Build](https://github.com/DanielKirshner/Windows-PE-Loader/actions/workflows/windows-vs-build.yml/badge.svg)

Windows-PE-Loader is a lightweight C-based library designed to parse and load Windows Portable Executable (PE) files directly from memory. Developed as part of OS internals research, this project provides insights into the PE file structure and the loading process, including handling of import tables, relocations, and more.

## Table of Contents
- [Features](#features)
- [C++ Usage](#usage)
- [Logs](#logs)
- [License](#license)

## Features

- **PE Header Parsing**  
  Parses DOS, NT, and section headers to extract and validate core metadata necessary for loading.

- **Section Mapping**  
  Maps sections from the PE file into memory with correct protections (e.g., executable, writable) and alignment.

- **Import Table Handling**  
  Parses the Import Directory Table and loads dependencies using `LoadLibrary`, resolving imported symbols with `GetProcAddress`. Handles both ILT and IAT.

- **Relocation Processing**  
  Adjusts absolute addresses when the module is loaded at a base address different from its preferred one.
  Supports applying relocation blocks.

- **Memory-Based Execution**  
  Loads and executes PE modules directly from memory buffers, ideal for reflective loading scenarios.

- **Dynamic Library Management**  
  Provides an abstraction for loading and freeing in-memory libraries using:

- **Minimal Dependencies**
  Designed with no external dependencies beyond the Windows API.

## Usage
```c++
extern "C"
{
#include "pe_library.h"
}

#include <Windows.h>

int32_t APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int32_t)
{
        const HMODULE module = Library__load_library(
            DLL_BUFFER_DATA,
            DLL_BUFFER_SIZE
        );

        Library__FreeLibrary(module);
}
```

## Logs

- Logging is enabled automatically in debug builds and provides useful runtime information.

- To view logs, run the application in a debugger (e.g., Visual Studio) or use [DbgView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) from Sysinternals.

- Logs are not available in release builds.

## License
This project is licensed under the MIT License.
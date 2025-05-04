# Windows-PE-Loader
Windows Portable Executable parser and loader library implementation as part of my Windows OS internals research.

## C++ Usage
```c
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

## 📜 License
This project is licensed under the MIT License.
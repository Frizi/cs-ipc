#include "macros.h"

#include <string>
#include <cstring>
#include <windows.h>

namespace CsIpc
{
    std::wstring GetPublicPipename(const char* name)
    {
        std::wstring pipename = L"\\\\.\\pipe\\csipc\\";
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, name, strlen(name), NULL, 0);
        wchar_t *buffer = new wchar_t[bufferSize+2];
        memset(buffer, 0, bufferSize+2);
        MultiByteToWideChar(CP_UTF8, 0, name, strlen(name), buffer, bufferSize);
        pipename += buffer;
        delete[] buffer;
        return pipename; //copy return
    }
}

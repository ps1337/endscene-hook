#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <map>

#include "injectedThread.h"

// the main method of the injected dll
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)injectedThread, hModule, 0, 0));
        break;
    }
    return TRUE;
}
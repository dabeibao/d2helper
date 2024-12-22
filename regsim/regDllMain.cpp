#include <windows.h>
#include "regsim.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        RegsimInit();
        return TRUE;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (!lpReserved) {
            RegsimRelease();
        }
        break;
    }
    return TRUE;
}

// force generate .lib (to avoid building d2reg every time)
extern "C" __declspec(dllexport) void Init()
{
}

#include <windows.h>
#include "Install.h"
#include "log.h"

#ifdef REGSIM_ON
#include "regsim.h"
#endif

HMODULE hInstDLL=NULL;

#ifdef REGSIM_ON
static inline void installRegsim()
{
    RegsimInit();
}
static inline void uninstallRegsim()
{
    RegsimRelease();
}
#else
static inline void installRegsim()
{
}
static inline void uninstallRegsim()
{
}
#endif

static void dllInit()
{
    Install();
    installRegsim();
}

static void dllRelease()
{
    uninstallRegsim();
    Uninstall();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hInstDLL = hModule;
        DisableThreadLibraryCalls(hInstDLL);
        dllInit();
        return TRUE;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (!lpReserved) {
            dllRelease();
        }
        break;
    }
    return TRUE;
}

extern "C" PVOID __stdcall QueryInterface() //NOTE :- needs to be exported
{
        log_trace("API called\n");
	return NULL;
}

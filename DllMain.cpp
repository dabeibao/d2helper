#include <windows.h>
#include "Install.h"
#include "log.h"

HMODULE hInstDLL=NULL;

void ReleaseMem()
{
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstDLL = hModule;
		DisableThreadLibraryCalls(hInstDLL);
		return Install();
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		ReleaseMem();
		if (!lpReserved)
			Uninstall();
		break;
	}
	return TRUE;
}

extern "C" PVOID __stdcall QueryInterface() //NOTE :- needs to be exported
{
        log_trace("API called\n");
	return NULL;
}

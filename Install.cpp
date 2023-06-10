#include <windows.h>
#include <stdio.h>

#include "d2ptrs.h"
#include "HelpFunc.h"
#include "Install.h"
#include "log.h"
#include "config.hpp"
#include "loader.hpp"
#include "d2h_module.hpp"
#include "D2Utils.hpp"
#include "helper.h"

struct d2ptr {
    const char *        name;
    DWORD *             ptr;
};

#pragma comment(lib, "Version.Lib")

extern void ItemInGround_patch(const config_t * config);

static BOOL fDefault = TRUE;
// static Patch_t aD2Patchs[] = {
//     {PatchJMP, DLLOFFSET2(D2CLIENT, 0x6FB34BA4, 0x6FB6C284), (DWORD)(uintptr_t)RecvCommand_PickUp_Patch_ASM, 5, &fDefault},
// };

#define D2FUNCPTR(d1,o1,v1,t1,t2)       {"D2" # v1, (DWORD *) &(D2##v1), },
#define D2FUNCPTR2(d1,o1,o2,v1,t1,t2)   {"D2" # v1, (DWORD *) &(D2##v1), },
#define D2VARPTR(d1,o1,v1,t1)           {"p_D2" # v1, (DWORD *) &(p_D2##v1), },
#define D2VARPTR2(d1,o1,o2,v1,t1)       {"p_D2" # v1, (DWORD *) &(p_D2##v1), },
#define D2ASMPTR(d1,o1,v1)              {"vD2" # v1, (DWORD *) &(vD2##v1), },
#define D2ASMPTR2(d1,o1,o2,v1)          {"vD2" # v1, (DWORD  *) &(vD2##v1), },

static d2ptr ptrs[] = {
#include "d2ptrs.inc"
};


BOOL RelocD2Ptrs()
{
    for (int i = 0; i < _ARRAYSIZE(ptrs); i += 1) {
        struct d2ptr *  ptr = &ptrs[i];
        DWORD           value = *(ptr->ptr);
        DWORD           new_offset = GetDllOffset(value);
        log_verbose("Reloc %s addr %p 0x%x -> 0x%x\n", ptr->name, ptr->ptr, value, new_offset);
        log_flush();
        *(ptr->ptr) = new_offset;
    }

    return true;
}

BOOL installPatch(Patcher * patch)
{
    DWORD       addr;
    if (patch->isInited) {
        return true;
    }
    log_verbose("Patching: dll %s, offset 0x%x\n", patch->dllName, patch->offset);
    addr = GetDllOffset(patch->dllName, patch->offset);
    if (addr == 0) {
        log_warn("cannot get dll %s offset %u\n", patch->dllName, patch->offset);
        return false;
    }
    patch->isInited = true;
    patch->patchedAddr = addr;
    patch->oldCode = new BYTE[patch->len];
    ReadLocalBYTES(addr, patch->oldCode, patch->len);
    patch->func(addr, patch->param, patch->len);

    return true;
}

void uninstallPatch(Patcher * patch)
{
    if (!patch->isInited) {
        return;
    }
    if (patch->oldCode == nullptr) {
        return;
    }
    WriteLocalBYTES(patch->patchedAddr, patch->oldCode, patch->len);
    delete patch->oldCode;
}

BOOL InstallD2Patchs(Patch_t* pPatchStart, Patch_t* pPatchEnd)
{
  Patch_t* p = pPatchStart;
  while( p<pPatchEnd ){
      if ( !p->fInit){
          p->addr = GetDllOffset(p->addr);
          p->fInit = 1 ;
      }
      log_verbose("Patching: func %p, en %u addr 0x%x version 0x%x\n", p->func, *(p->fEnable), p->addr, Map_D2Version());
      if (p->func && *(p->fEnable)){
          p->oldcode = new BYTE[p->len];
          ReadLocalBYTES(p->addr, p->oldcode, p->len);
          p->func(p->addr, p->param, p->len);
      }
      p++;
  }
  return TRUE;
}

void RemoveD2Patchs(Patch_t* pPatchStart, Patch_t* pPatchEnd)
{
  Patch_t* p = pPatchStart;
  while( p<pPatchEnd) {
    if (p->oldcode && *(p->fEnable) ){
      WriteLocalBYTES(p->addr, p->oldcode, p->len);
      delete p->oldcode;
    }
    p++;
  }
}

static bool loadConfig(config_t * config)
{
    char        path[512];
    char        fullName[sizeof(path) + 32];
    bool        ok;
    
    ok = getAppDirectory(path, sizeof(path));
    if (!ok) {
        log_trace("Cannot get APP path\n");
        return false;
    }
    snprintf(fullName, sizeof(fullName), "%s\\%s", path, INI_FILE_NAME);
    config_load(fullName, config);
    log_verbose("Load Config: %d, %d, %d\n", config->is_enable, config->is_verbose, config->bag_item);

    return true;
}

static void onLoad_ASM();

static Patcher loadPatch {
    PatchCALL, "D2WIN.dll", 0x6F8F8824 - DLLBASE_D2WIN, (DWORD)(uintptr_t)onLoad_ASM, 5,
};

static int isLoaded;
static void onLoad()
{
    log_trace("Game created\n");
    isLoaded = 1;
    module_on_load();
}

//static void *origCode;

static void __declspec(naked) onLoad_ASM()
{
    __asm {
        cmp     [isLoaded], 1
        je      done

        push    edi
        call    onLoad
        pop     edi

    done:
        pop     edx     // the orig address

    // orig:
        lea     ecx, dword ptr[esp+0xC]
        push    ecx

    // jmp back
        jmp     edx
    }
}

#if 0
static void patchCreate()
{
    DWORD       offset = DLLOFFSET(D2WIN, 0x6F8F8824);
    DWORD       addr = GetDllOffset(offset);

    if (addr == 0) {
        log_warn("Patch create address failed, offset 0x%x\n", offset);
        return;
    }
    origCode = PatchDetour(addr, (DWORD)(uintptr_t)onLoad_ASM, 5);
}
#endif


DWORD CalcFuncOffset_X(DWORD dwNo, DWORD dwBase, int iValue)
{
    if ( 0 > iValue )
    {
        return (dwNo | (iValue <<8));
    }

    return (dwNo | ((iValue - dwBase) <<8));
}


BOOL Install()
{
    config_t    config;

    log_init();
    if (!loadConfig(&config)) {
        return FALSE;
    }

    if (config.is_verbose) {
        log_set_verbose();
    }
    D2Util::setVerbose(config.is_verbose);
    D2Util::setDebug(config.is_debug);

    log_verbose("Reload ptrs\n");
    if(!RelocD2Ptrs()){
        return FALSE;
    }

    module_init();
    installPatch(&loadPatch);
    log_verbose("init done\n");

    return TRUE;
}

void Uninstall()
{
    uninstallPatch(&loadPatch);
    module_destroy();
    log_verbose("Unloaded\n");
    log_release();
}

int helper_install()
{
    return Install();
}

void helper_uninstall()
{
    return Uninstall();
}

#if 0
DWORD GetD2Version(LPCVOID pVersionResource)
{
	if (!pVersionResource) return D2_VER_113C;

	UINT uLen;
	VS_FIXEDFILEINFO* ptFixedFileInfo;
	if (!VerQueryValue(pVersionResource, "\\", (LPVOID*)&ptFixedFileInfo, &uLen))
		return D2_VER_113C;

	if (uLen == 0)
		return D2_VER_113C;

	WORD major = HIWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD minor = LOWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD revision = HIWORD(ptFixedFileInfo->dwFileVersionLS);
	WORD subrevision = LOWORD(ptFixedFileInfo->dwFileVersionLS);

	if (major != 1)
		return D2_VER_113C;
	if (minor == 0 && revision == 13 && subrevision == 64) return D2_VER_113D;
	return D2_VER_113C;
}

DWORD GetD2Version(char* gameExe)
{
    static DWORD version = 0xFFFFFFFF;

    if ( 0xFFFFFFFF != version )
    {
        return version;
    }

	DWORD len = GetFileVersionInfoSize(gameExe, NULL);
	if (len == 0)
	{
	    version = D2_VER_113C;
		return version;
	}

	BYTE* pVersionResource = new BYTE[len];
	GetFileVersionInfo(gameExe, NULL, len, pVersionResource);
	version = GetD2Version(pVersionResource);
	delete pVersionResource;

	return version;
}

DWORD Map_D2Version()
{
    char szBuffer[MAX_PATH] = {0};
    char *pcTemp = NULL;
    static DWORD version = 0xFFFFFFFF;

    if ( 0xFFFFFFFF != version )
    {
        return version;
    }

    GetModuleFileName(GetModuleHandle("PlugY.dll"), szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]) - 1);
    if ( 0 == szBuffer[0] || NULL == (pcTemp = strstr(szBuffer, ".dll")) )
	{
	    version = D2_VER_113C;
		return version;
	}

    pcTemp -= strlen("PlugY");
    strcpy(pcTemp, "game.exe");
    return (version = GetD2Version(szBuffer));
}
#else
eGameVersion GetD2Version(LPCVOID pVersionResource)
{
	if (!pVersionResource) return UNKNOWN;

	UINT uLen;
	VS_FIXEDFILEINFO* ptFixedFileInfo;
	if (!VerQueryValue(pVersionResource, "\\", (LPVOID*)&ptFixedFileInfo, &uLen))
		return UNKNOWN;

	if (uLen == 0)
		return UNKNOWN;

	WORD major = HIWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD minor = LOWORD(ptFixedFileInfo->dwFileVersionMS);
	WORD revision = HIWORD(ptFixedFileInfo->dwFileVersionLS);
	WORD subrevision = LOWORD(ptFixedFileInfo->dwFileVersionLS);

	if (major != 1)
		return UNKNOWN;
	if (minor == 0 && revision == 7 && subrevision == 0) return UNKNOWN;//V107;
	if (minor == 0 && revision == 8 && subrevision == 28) return UNKNOWN;//V108;
	if (minor == 0 && revision == 9 && subrevision == 19) return UNKNOWN;//V109;
	if (minor == 0 && revision == 9 && subrevision == 20) return UNKNOWN;//V109b;
	if (minor == 0 && revision == 9 && subrevision == 22) return UNKNOWN;//V109d;
	if (minor == 0 && revision == 10 && subrevision == 39) return UNKNOWN;//V110;
	if (minor == 0 && revision == 11 && subrevision == 45) return UNKNOWN;//V111;
	if (minor == 0 && revision == 11 && subrevision == 46) return UNKNOWN;//V111b;
	if (minor == 0 && revision == 12 && subrevision == 49) return UNKNOWN;//V112;
	if (minor == 0 && revision == 13 && subrevision == 60) return D2_VER_113C;
	if (minor == 0 && revision == 13 && subrevision == 64) return D2_VER_113D;
	if (minor == 14 && revision == 0 && subrevision == 64) return UNKNOWN;//V114a;
	if (minor == 14 && revision == 1 && subrevision == 68) return UNKNOWN;//V114b;
	if (minor == 14 && revision == 2 && subrevision == 70) return UNKNOWN;//V114c;
	if (minor == 14 && revision == 3 && subrevision == 71) return UNKNOWN;//V114d;
	return UNKNOWN;
}

eGameVersion GetD2Version(char* gameExe)
{
    static eGameVersion version = UNKNOWN;

    if ( UNKNOWN != version )
    {
        return version;
    }

	DWORD len = GetFileVersionInfoSize(gameExe, NULL);
	if (len == 0)
		return UNKNOWN;

	BYTE* pVersionResource = new BYTE[len];
	GetFileVersionInfo(gameExe, NULL, len, pVersionResource);
	version = GetD2Version(pVersionResource);
	delete[] pVersionResource;

	return version;
}

eGameVersion GetD2Version(HMODULE hModule)
{
    static eGameVersion version = UNKNOWN;

    if ( UNKNOWN != version )
    {
        return version;
    }

	HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (!hResInfo) return UNKNOWN;
	HGLOBAL hResData = LoadResource(hModule, hResInfo);
	if (!hResData) return UNKNOWN;
	LPVOID pVersionResource = LockResource(hResData);
	version = GetD2Version(pVersionResource);
	FreeResource(hResData);
	return version;
}

DWORD Map_D2Version()
{
    char szBuffer[MAX_PATH] = {0};
    char *pcTemp = NULL;
    static eGameVersion version = UNKNOWN;

    if ( UNKNOWN != version )
    {
        return version;
    }

    version = GetD2Version((HMODULE)GetModuleHandle(NULL));

    return version;
}
#endif


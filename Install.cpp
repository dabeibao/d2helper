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


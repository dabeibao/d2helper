#ifndef INSTALL_H
#define INSTALL_H

#include <windows.h>


struct Patch_t {
    void (*func)(DWORD, DWORD, DWORD);
    DWORD addr;
    DWORD param;
    DWORD len;
    BOOL *fEnable;
    BYTE *oldcode;
    BOOL fInit;
};

struct Patcher {
    void (*func)(DWORD, DWORD, DWORD);

    const char *        dllName;
    int                 offset;

    DWORD               param;
    int                 len;

    BYTE *              oldCode;
    bool                isInited;
    DWORD               patchedAddr;
};


BOOL Install();
void Uninstall();
BOOL LoadConfig();
void ReloadConfig();
void ShowWarningMessage();
BOOL InstallD2Patchs(Patch_t* pPatchStart, Patch_t* pPatchEnd);
void RemoveD2Patchs(Patch_t* pPatchStart, Patch_t* pPatchEnd);
void InstallPatchAfterLoad_ASM();
void InitCellFiles();
void DeleteCellFiles();
BOOL installPatch(Patcher * patch);
void uninstallPatch(Patcher * patch);

#endif

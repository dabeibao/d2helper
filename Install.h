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



#endif

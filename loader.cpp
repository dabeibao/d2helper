#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "loader.hpp"
#include "log.h"

bool getDllPath(char *path, int size)
{
    HMODULE     hm;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&getDllPath, &hm)) {
        return false;
    }
    if (!GetModuleFileNameA(hm, path, size)) {
        return false;
    }

    return true;
}

bool getAppDirectory(char *path, int size)
{
    char *      div;
    bool        ok;

    ok = getDllPath(path, size);
    if (!ok) {
        return false;
    }

    div = strrchr(path, '\\');
    if (div != NULL) {
        *div = '\0';
    }
    return true;
}

HMODULE tryLoadDll(const char *name)
{
    char                path[512];
    char                dllPath[512];
    HMODULE             hmodule;

    hmodule = LoadLibraryA(name);
    if (hmodule) {
        return hmodule;
    }

    if (!getAppDirectory(path, sizeof(path))) {
        return nullptr;
    }
    log_verbose("App path %s\n", path);
    snprintf(dllPath, sizeof(dllPath), "%s\\%s", path, name);

    hmodule = LoadLibraryA(dllPath);
    return hmodule;
}

#include <vector>
#include <windows.h>
#include <shlwapi.h>

#include "../detours/detours.h"
#include "regsim.h"
#include "KeyMap.hpp"
#include "RegIni.hpp"
#include "loader.hpp"
#include "cConfigLoader.hpp"
#include "log.h"
#include "strutils.hpp"

#pragma comment(lib, "shlwapi.lib")

#define REG_SIM_PRESET  "D2RegSimPre.ini"
#define REG_SIM_FILE    "D2RegSim.ini"

extern "C" {
LONG (WINAPI * Real_RegOpenKeyExA)(HKEY a0,
                                   LPCSTR a1,
                                   DWORD a2,
                                   REGSAM a3,
                                   PHKEY a4) = RegOpenKeyExA;

LONG (WINAPI * Real_RegOpenKeyExW)(HKEY a0,
                                   LPCWSTR a1,
                                   DWORD a2,
                                   REGSAM a3,
                                   PHKEY a4) = RegOpenKeyExW;

LONG (WINAPI * Real_RegCreateKeyExA)(HKEY a0,
                                     LPCSTR a1,
                                     DWORD a2,
                                     LPSTR a3,
                                     DWORD a4,
                                     REGSAM a5,
                                     LPSECURITY_ATTRIBUTES a6,
                                     PHKEY a7,
                                     LPDWORD a8) = RegCreateKeyExA;

LONG (WINAPI * Real_RegSetValueExA)(HKEY a0,
                                    LPCSTR a1,
                                    DWORD a2,
                                    DWORD a3,
                                    const BYTE* a4,
                                    DWORD a5) = RegSetValueExA;

LONG (WINAPI * Real_RegQueryValueExA)(HKEY a0,
                                      LPCSTR a1,
                                      LPDWORD a2,
                                      LPDWORD a3,
                                      LPBYTE a4,
                                      LPDWORD a5) = RegQueryValueExA;

LONG (WINAPI * Real_RegCloseKey)(HKEY a0) = RegCloseKey;


}

#define BLZ_KEY         "SOFTWARE\\Blizzard Entertainment\\Diablo II"
#define BLZ_KEY_W       L"SOFTWARE\\Blizzard Entertainment\\Diablo II"

static RegIni * gRegIni;
static bool regFallbackEnabld = false;

static bool isBlzKey(HKEY hKey, LPCSTR subKey)
{
    if (hKey != HKEY_CURRENT_USER) {
        return false;
    }
    if (_stricmp(subKey, BLZ_KEY) != 0) {
        return false;
    }

    return true;
}

static bool isBlzKeyW(HKEY hKey, LPCWSTR subKey)
{
    if (hKey != HKEY_CURRENT_USER) {
        return false;
    }
    if (_wcsicmp(subKey, BLZ_KEY_W) != 0) {
        return false;
    }

    return true;
}

static bool isAllowQueryRedirect(const char * key)
{
    if (!regFallbackEnabld) {
        return false;
    }

    static const char * notAllowKeys[] = {
        "InstallPath", "Save Path",
        "Preferred Realm",
    };

    for (auto sk: notAllowKeys) {
        if (_stricmp(sk, key) == 0) {
            return false;
        }
    }

    return true;
}

static LONG WINAPI Mine_RegOpenKeyExA(HKEY      hKey,
                                      LPCSTR    subKey,
                                      DWORD     ulOptions,
                                      REGSAM    samDesired,
                                      PHKEY     outKey)
{
    log_debug("%s entered\n", __FUNCTION__);
    LONG rv = Real_RegOpenKeyExA(hKey, subKey, ulOptions, samDesired, outKey);

    if (rv != ERROR_SUCCESS) {
        return rv;
    }
    if (!isBlzKey(hKey, subKey)) {
        return rv;
    }

    log_verbose("BLZ RegOpenKeyExA, key %p pool %u\n", *outKey, KeyMap::inst().size());
    KeyMap::inst().add(*outKey);

    return rv;
}

LONG WINAPI Mine_RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    log_debug("%s entered\n", __FUNCTION__);
    LONG rv = Real_RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);

    if (rv != ERROR_SUCCESS) {
        return rv;
    }
    if (!isBlzKeyW(hKey, lpSubKey)) {
        return rv;
    }
    KeyMap::inst().add(*phkResult);
    log_verbose("BLZ RegOpenKeyExW, key %p\n", *phkResult);
    return rv;
}

LONG WINAPI Mine_RegCreateKeyExA(HKEY           hKey,
                                 LPCSTR         lpSubKey,
                                 DWORD          Reserved,
                                 LPSTR          lpClass,
                                 DWORD          dwOptions,
                                 REGSAM         samDesired,
                                 const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                 PHKEY          phkResult,
                                 LPDWORD        lpdwDisposition)
{
    LONG        rv;

    log_debug("%s entered\n", __FUNCTION__);
    rv = Real_RegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired,
                              lpSecurityAttributes, phkResult, lpdwDisposition);
    if (rv != ERROR_SUCCESS) {
        return rv;
    }
    if (!isBlzKey(hKey, lpSubKey)) {
        return rv;
    }
    KeyMap::inst().add(*phkResult);
    log_verbose("RegCreateKeyExA from BLZ key %p\n", *phkResult);

    return rv;
}

LONG WINAPI Mine_RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
                                DWORD dwType, const BYTE *lpData, DWORD cbData)
{
    LONG        rv;

    log_debug("%s entered\n", __FUNCTION__);
    if (KeyMap::inst().hasKey(hKey)) {
        rv = gRegIni->save(lpValueName, dwType, lpData, cbData);
        log_verbose("RegSetValueExA from BLZ rv %lu key %p value %hs type %lu size %lu\n",
                    rv, hKey, lpValueName, dwType, cbData);
        if (rv == ERROR_SUCCESS) {
            return rv;
        }

        // fallback to real registry
        log_verbose("RegSetValueExA key %p value %hs fallback\n", hKey, lpValueName);
    }

    rv = Real_RegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);

    log_debug("RegSetValueExA key %p value %hs type %lu size %lu\n",
              hKey, lpValueName, dwType, cbData);

    return rv;
}

LONG WINAPI Mine_RegCloseKey(HKEY hKey)
{
    LONG        rv;

    log_debug("%s entered\n", __FUNCTION__);
    if (KeyMap::inst().hasKey(hKey)) {
        KeyMap::inst().remove(hKey);
        log_verbose("RegCloseKey from Blz key %p pool %u\n", hKey, KeyMap::inst().size());
    }
    rv = Real_RegCloseKey(hKey);
    log_debug("RegCloseKey key %p, pool %u\n", hKey, KeyMap::inst().size());

    return rv;
}

#define getInt(p)       (p == NULL? 0 : *p)

LONG WINAPI Mine_RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved,
                                  LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    LONG        rv;

    log_debug("%s entered\n", __FUNCTION__);
    if (KeyMap::inst().hasKey(hKey)) {
        DWORD   size = (lpcbData == NULL || lpData == NULL)? 0 : *lpcbData;
        DWORD   outSize = 0;

        rv = gRegIni->load(lpValueName, lpData, size, &outSize, lpType);
        log_verbose("RegQueryValueExA from BLZ rv %lu key %p value %s type %lu data %lu(%lu)\n",
                  rv, hKey, lpValueName, getInt(lpType), getInt(lpcbData), outSize);

        if (rv != ERROR_FILE_NOT_FOUND || !isAllowQueryRedirect(lpValueName)) {
            if (lpcbData != NULL) {
                *lpcbData = outSize;
            }

            // The win32 API design. If data is null, it means caller is querying the size
            if (rv == ERROR_MORE_DATA && lpData == NULL) {
                rv = ERROR_SUCCESS;
            }
            return rv;
        }

        // fallback to default
        log_verbose("RegQueryValueExA key %p value %hs fallback\n", hKey, lpValueName);
    }

    rv = Real_RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    log_debug("RegQueryValueExA key %p value %hs type %lu data size %lu rv %lu\n",
              hKey, lpValueName, getInt(lpType), getInt(lpcbData), rv);

    return rv;
}

#define ATTACH(x)       DetourAttach(&(PVOID&)Real_##x, (PVOID)Mine_##x)
#define DETACH(x)       DetourDetach(&(PVOID&)Real_##x, (PVOID)Mine_##x)

static void attachDetours()
{
    log_verbose("Install Detours\n");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    ATTACH(RegOpenKeyExA);
    ATTACH(RegOpenKeyExW);
    ATTACH(RegCreateKeyExA);
    ATTACH(RegSetValueExA);
    ATTACH(RegQueryValueExA);
    ATTACH(RegCloseKey);

    DetourTransactionCommit();
    log_verbose("Install Detours done\n");
}

static void detachDetours()
{
    log_verbose("Uninstall Detours\n");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DETACH(RegOpenKeyExA);
    DETACH(RegOpenKeyExW);
    DETACH(RegCreateKeyExA);
    DETACH(RegSetValueExA);
    DETACH(RegQueryValueExA);
    DETACH(RegCloseKey);

    DetourTransactionCommit();
}

static bool regsimIsInited;

// support -regdir in command line
static void getRegDir(char *outPath, int size, bool enableCommandLine)
{
    getAppDirectory(outPath, size);

    if (!enableCommandLine) {
        return;
    }
    char* cmdLine = GetCommandLineA();
    if (!cmdLine) {
        return;
    }

    // Copy command line to a modifiable buffer
    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%s", cmdLine);

    std::vector<std::string> list;
    helper::split(buffer, ' ', list);
    for (size_t i = 0; i < list.size() - 1; i += 1) {
        if (_stricmp(list[i].c_str(), "-regdir")) {
            continue;
        }

        auto& dir = list[i + 1];
        if (PathIsRelativeA(dir.c_str())) {
            char tmp[MAX_PATH];
            snprintf(tmp, sizeof(tmp), "%s\\%s", outPath, dir.c_str());
            snprintf(outPath, size, "%s", tmp);
            log_verbose("%d: %s, dir %s out %s\n", i, list[i].c_str(), dir.c_str(), outPath);
        } else {
            snprintf(outPath, size, "%s", dir.c_str());
        }
        for (int i = strlen(outPath) - 1; i >= 0; i -= 1) {
            if (outPath[i] != '\\') {
                break;
            }
            outPath[i] = '\0';
        }
        break;
    }
}

extern "C" int RegsimInit()
{
    if (regsimIsInited) {
        return 0;
    }

    auto section = CfgLoad::section("helper.regsim");
    bool enabled = section.loadBool("enable", true);

    if (!enabled) {
        return 0;
    }

    regFallbackEnabld = section.loadBool("enableFallback", regFallbackEnabld);

    char        path[512];
    char        prePath[512 + 32];
    char        fullName[512 + 32];

    log_trace("Regsim load\n");
    regsimIsInited = true;

    bool        enableCommandLine = section.loadBool("enableCommandLine", true);

    getRegDir(path, sizeof(path), enableCommandLine);

    log_verbose("REGSIM: use registry folder %s, cmd: %d\n", path, enableCommandLine);
    snprintf(prePath, sizeof(prePath), "%s\\%s", path, REG_SIM_PRESET);
    snprintf(fullName, sizeof(fullName), "%s\\%s", path, REG_SIM_FILE);

    gRegIni = new RegIni(prePath, fullName);
    attachDetours();

    return 0;
}

extern "C" void RegsimRelease()
{
    if (!regsimIsInited) {
        return;
    }
    log_trace("Regsim Unload\n");
    detachDetours();
    delete gRegIni;
}

#include <windows.h>
#include <commctrl.h>
#include "Define.h"
#include "d2h_module.hpp"
#include "d2ptrs.h"
#include "log.h"
#include "config.hpp"
#include "macros.hpp"
#include "D2CallStub.hpp"
#include "d2vars.h"
#include "D2Utils.hpp"
#include "keyModule.hpp"
#include "Event.hpp"
#include "WinTM.hpp"

#pragma comment(lib, "Comctl32.lib")

#define SUB_ID  (0x13768)

struct KeyConfig {
    const char *        configName;
    int                 key;
    bool                (*func)(KeyConfig * config);
};

static void keydownPatch_ASM();
static bool doTransmute(KeyConfig * config);

// dll base: 5CC10000
// CPU Disasm
// Address   Hex dump          Command                                  Comments
// 5CC19300  /.  8A4F 08       MOV CL,BYTE PTR DS:[EDI+isInSleepCall]
// 5CC19303  |.  8A57 0F       MOV DL,BYTE PTR DS:[EDI+0F]
// 5CC19306  |.  80E2 40       AND DL,40
// 5CC19309  |.  E8 32FDFFFF   CALL 5CC19040
// 5CC1930E  |.  F647 0F 40    TEST BYTE PTR DS:[EDI+0F],40
// 5CC19312  \.  C3            RETN

// static Patcher keyEntryPatch = {
//     PatchJMP, "d2hackmap.dll", 0x930e, (DWORD)keydownPatch_ASM, 5,
// };

static KeyConfig keyConfigs[] = {
    { "transmute", 0, doTransmute,}
};

static KeyFunc keyPreFunc[8];

static KeyConfig* validKeyConfigs[1024];

#define D2TXTCODE(x) ( ((x)&0xff)<<24 | ((x)>>8&0xff)<<16 | ((x)>>16&0xff)<<8 | ((x)>>24&0xff) )

static DWORD blockMouseStart;
static bool doTransmute(KeyConfig * )
{
    if (PLAYER == nullptr || PLAYER->pInventory == nullptr) {
        return false;
    }
    if (GetTickCount() - blockMouseStart < 100) {
        return false;
    }
    DWORD v = D2CheckUiStatus(UIVAR_CUBE);

    log_verbose("CUBE is open: %d\n", v);
    if (v == 0) {
        return false;
    }
    UnitAny * item = PLAYER->pInventory->pCursorItem;
    if (item != nullptr) {
        D2Util::showVerbose(L"Some Item On HAND!");
        return false;
    }
    D2Transmute();
    //BYTE packet[7] = { 0x4f, 0x18, };
    //D2SendPacket(sizeof(packet), 1, packet);
    return true;
}

static bool keyDownPatch(BYTE keyCode, BYTE repeat)
{
    log_verbose("key down called, player %p\n", PLAYER);

    for (int i = 0; i < ARRAY_SIZE(keyPreFunc); i += 1) {
        if (keyPreFunc[i] == nullptr) {
            break;
        }
        bool    blocked = keyPreFunc[i](keyCode, repeat);
        if (blocked) {
            D2Util::showVerbose(L"Blocked");
            return blocked;
        }
    }

    if (repeat) {
        return false;
    }

    for (auto kc: validKeyConfigs) {
        if (kc == nullptr) {
            break;
        }
        if (kc->key == keyCode) {
            bool ok = kc->func(kc);
            if (ok) {
                return true;
            }
        }
    }
    return false;
}

static void loadKeyConfig()
{
    int         validCount = 0;
    Config      cfg("key");

    for (int i = 0; i < ARRAY_SIZE(keyConfigs); i += 1) {
        KeyConfig *     kc = &keyConfigs[i];
        int     key = cfg.load(kc->configName);
        if (key == 0) {
            continue;
        }
        log_verbose("Map key %d to %s\n", key, kc->configName);
        kc->key = key;
        validKeyConfigs[validCount] = kc;
        validCount += 1;
        if (validCount >= ARRAY_SIZE(validKeyConfigs) - 1) {
            break;
        }
    }
    log_verbose("Valid Keys %d\n", validCount);
    validKeyConfigs[validCount] = nullptr;
}

HWND origD2Hwnd;

static int keyModuleInstall()
{
    // const char * dllName = getMapDllName();
    // keyEntryPatch.dllName = dllName;
    // BOOL ok = installPatch(&keyEntryPatch);
    // if (!ok) {
    //     log_warn("Failed to patch key entry\n");
    //     return -1;
    // }

    loadKeyConfig();

    return 0;
}

static void keyModuleUninstall()
{
    //uninstallPatch(&keyEntryPatch);
}


LRESULT CALLBACK keyModuleEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                UINT_PTR uidsubclass, DWORD_PTR dwRefData)
{
    static bool isInGame;

    if (!D2Util::inGame()) {
        if (isInGame) {
            isInGame = false;
            Event::trigger(Event::GameEnd);
            //WinTM::inst().onGameStop();
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (!isInGame) {
        isInGame = true;
        Event::trigger(Event::GameStart);
        D2ShowMap();
        //WinTM::inst().onGameStart();
    }

    bool        blocked = false;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        blockMouseStart = GetTickCount();
        break;
    case WM_TIMER:
        WinTM::inst().onTimer(wParam, &blocked);
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        blocked = keyDownPatch(wParam, (lParam >> 24) & 0x40);
        break;
    case WM_DESTROY:
        log_trace("WIN destroyed\n");
        RemoveWindowSubclass(hWnd, keyModuleEvent, SUB_ID);
        break;
    default:
        break;
    }
    if (blocked) {
        return 0;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static void keyModuleOnLoad()
{
    HWND        hWnd = D2GetHwnd();
    if (hWnd == nullptr) {
        log_warn("No window found\n");
        return;
    }

    log_trace("Window ready %p\n", hWnd);
    origD2Hwnd = hWnd;
    SetWindowSubclass(hWnd, keyModuleEvent,  SUB_ID, NULL);
}

static void keyModuleReload()
{
    loadKeyConfig();
}

void keyRegisterHandler(KeyFunc func)
{
    for (int i = 0; i < ARRAY_SIZE(keyPreFunc); i += 1) {
        if (keyPreFunc[i] == nullptr) {
            keyPreFunc[i] = func;
            break;
        }
    }
}

D2HModule keyModule = {
    "Key Module",
    keyModuleInstall,
    keyModuleUninstall,
    keyModuleOnLoad,
    keyModuleReload,
};

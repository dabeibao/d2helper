#include "Define.h"
#include "Event.hpp"
#include "HelpFunc.h"
#include "PD2Support.hpp"
#include "D2Utils.hpp"
#include "d2ptrs.h"
#include "d2structs.h"
#include "log.h"
#include <cstdint>

bool PD2IsEnabled;
static double PD2Xscale = 1.0;
static double PD2YScale = 1.0;
static D2_KEY_TABLE * pd2KeyTab = nullptr;

static void PD2SetupScale()
{
    auto hWnd = D2GetHwnd();
    RECT rClient;
    GetClientRect(hWnd, &rClient);
    auto width = rClient.right - rClient.left;
    auto height = rClient.bottom - rClient.top;
    auto xScale = width * 1.0 / SCREENSIZE.x;
    auto yScale = height * 1.0 / SCREENSIZE.y;

    //TODO: is there case that the windows is smaller??
    PD2Xscale = min(xScale, 1.0);
    PD2YScale = min(yScale, 1.0);

    ////D2Util::showInfo(L"PD2Enabled, %p: x: %lf, y:%lf", pd2KeyTab, PD2Xscale, PD2YScale);
}

static DWORD mapToHwndPos(int x, int y)
{
    DWORD wx = (DWORD)(x * PD2Xscale);
    DWORD wy = (DWORD)(y * PD2YScale);

    return (wx | wy << 16);
}

static int getHoldKey()
{
    if (pd2KeyTab == nullptr) {
        return -1;
    }

    for (auto i = 0; i < 2; i += 1) {
        auto  keyMap = &pd2KeyTab[D2_KEY_HOLD_OFFSET + i];
        if (keyMap->key != 0xffff) {
            return keyMap->key;
        }
    }

    return -1;
}

int PD2GetFunc(unsigned char key)
{
    if (pd2KeyTab == nullptr) {
        return -1;
    }
    int func = D2Util::getKeyFuncFromTable(key, pd2KeyTab);
    ///D2Util::showInfo(L"%d -> %d", key, func);
    return func;
}


void NotifyKeyUp(HWND hwnd, WPARAM vk)
{
    UINT scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    UINT scanEx = MapVirtualKeyEx(vk, MAPVK_VK_TO_VSC_EX, GetKeyboardLayout(0));
    bool isExtended = (scanEx & 0xFF00) != 0;

    // for keyup, set bits 30 (was down) and 31 (transition)
    LPARAM lParamUp = (1)
                    | (scan  << 16)
                    | (isExtended ? (1 << 24) : 0)
                    | (1 << 30)
                    | (1 << 31);

    PostMessage(hwnd, WM_KEYUP, vk, lParamUp);
}

void PD2Click(PD2ClickType click, bool isShift, int x, int y)
{
    auto hWnd = D2GetHwnd();
    DWORD pos = mapToHwndPos(x, y);
    ////D2Util::showInfo(L"Click %d: (%d,%d) -> (%d,%d) shift: %d", x, y, pos & 0xffff, pos >> 16, isShift);
    if (click == PD2ClickType::Left) {
        int holdKey = 0;
        if (isShift) {
            holdKey = getHoldKey();
        }
        ////D2Util::showInfo(L"HOLD key: %d", holdKey);
        if (holdKey > 0) {
            SendMessageA(hWnd, WM_KEYDOWN, holdKey, 0);
        }
        SendMessageA(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
        SendMessageA(hWnd, WM_LBUTTONUP, MK_LBUTTON, pos);
        if (holdKey > 0) {

            SendMessageA(hWnd, WM_KEYUP, holdKey, 0);
        }
    } else {
        SendMessageA(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, pos);
        SendMessageA(hWnd, WM_RBUTTONUP, MK_RBUTTON, pos);
    }
}

static bool PD2SetupHook(uintptr_t keyTabOffset)
{
    uintptr_t offset = GetDllOffset("ProjectDiablo.dll", keyTabOffset);
    if (offset == 0) {
        return false;
    }
    auto ptr = *(uintptr_t *)offset;
    pd2KeyTab = (D2_KEY_TABLE *)ptr;
    ///log_trace("PD2 key tab: %p\n", pd2KeyTab);

    static bool isAdded;
    if (!isAdded) {
        Event::add(Event::GameStart, [](Event::Type, DWORD, DWORD) {
            PD2SetupScale();
        });
        isAdded = true;
    }

    return true;
}

void PD2Setup(uintptr_t keyTableOffset)
{
    bool isEnable = keyTableOffset != 0;

    PD2IsEnabled = false;
    if (!isEnable) {
        return;
    }

    PD2IsEnabled = false;
    bool ok = PD2SetupHook(keyTableOffset);
    if (!ok) {
        return;
    }
    PD2IsEnabled = true;
}

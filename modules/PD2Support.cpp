#include <cstdint>
#include "Define.h"
#include "Event.hpp"
#include "HelpFunc.h"
#include "PD2Support.hpp"
#include "D2Utils.hpp"
#include "d2ptrs.h"
#include "d2structs.h"
#include "log.h"

static const char * pd2dll = "ProjectDiablo.dll";

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
    uintptr_t offset = GetDllOffset(pd2dll, keyTabOffset);
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

constexpr size_t strlen_const(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

static uintptr_t PD2FindKeyTableOffset()
{
    // 10185330  /.  55            PUSH EBP
    // 10185331  |.  8BEC          MOV EBP,ESP
    // 10185333  |.  53            PUSH EBX
    // 10185334  |.  8B5D 08       MOV EBX,DWORD PTR SS:[ARG.1]
    // 10185337  |.  F743 0C 00000 TEST DWORD PTR DS:[EBX+0C],40000000
    // 1018533E  |.  0F85 B9000000 JNZ 101853FD
    // 10185344  |.  56            PUSH ESI
    // 10185345  |.  8B73 08       MOV ESI,DWORD PTR DS:[EBX+8]
    // 10185348  |.  33D2          XOR EDX,EDX
    // 1018534A  |.  57            PUSH EDI
    // 1018534B  |.  8B3D D4F63D10 MOV EDI,DWORD PTR DS:[103DF6D4]   <<< Store the pointer to key table
    // 10185351  |.  8BCF          MOV ECX,EDI
    static const BYTE pattern[] = {
        0x33, 0xD2,                             // XOR EDX,EDX
        0x57,                                   // PUSH EDI
        0x8B, 0x3d, 0x00, 0x00, 0x00, 0x00,     // MOV EDI,DWORD PTR DS:[103DF6D4]
        0x8B, 0xCF,                             // MOV ECX,EDI
    };
    static constexpr const char *mask = "xxxxx????xx";
    static_assert(strlen_const(mask) == sizeof(pattern), "Pattern and mask has different size");

    BYTE *base;
    size_t size;
    bool ok = GetDllInfo(pd2dll, &base, &size);
    if (!ok || base == nullptr) {
        log_trace("%s is not loaded\n", pd2dll);
        return 0;
    }

    size_t offset = 0;
    uintptr_t baseAddr = (uintptr_t)base;
    while (offset < size) {
        auto next = FindPattern(&base[offset], size - offset, pattern, mask, sizeof(pattern));
        if (next < 0) {
            log_trace("%s cannot find key table\n", pd2dll);
            return 0;
        }
        uintptr_t addr = *(uint32_t *)(baseAddr + offset + next + 5);
        if (addr > baseAddr && addr < baseAddr + size - sizeof(pattern)) {
            log_trace("%s find key table at 0x%x (0x%x + 0x%x)",
                      pd2dll, addr, (uint32_t)baseAddr, addr - baseAddr);
            return addr - baseAddr;
        }
        offset += next + sizeof(pattern);
    }

    return false;
}

void PD2Setup(uintptr_t keyTableOffset)
{
    if (keyTableOffset == 1) {
        keyTableOffset = PD2FindKeyTableOffset();
    }

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

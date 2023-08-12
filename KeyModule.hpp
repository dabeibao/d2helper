#pragma once

#include <Windows.h>
#include <stdint.h>
#include "d2h_module.hpp"

extern D2HModule keyModule;

typedef bool (* KeyFunc)(BYTE key, BYTE repeat);

struct HotkeyConfig {
    uint32_t            hotKey;
    bool                (*func)(struct HotkeyConfig * config);
    void *              ctx;
};

void keyRegisterHandler(KeyFunc func);
void keyRegisterHotkey(uint32_t hotkey, bool (*func)(struct HotkeyConfig * config), void * ctx);
extern HWND origD2Hwnd;

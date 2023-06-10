#pragma once

#include <Windows.h>
#include "d2h_module.hpp"

extern D2HModule keyModule;

typedef bool (* KeyFunc)(BYTE key, BYTE repeat);

void keyRegisterHandler(KeyFunc func);

extern HWND origD2Hwnd;

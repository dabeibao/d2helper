#pragma once

#include <windows.h>

HMODULE tryLoadDll(const char *name);
bool getAppDirectory(char *path, int size);

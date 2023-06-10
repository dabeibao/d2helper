#pragma once

#include <windows.h>

#define ORIG_MAP_NAME   "d2hackmap.dll"

HMODULE loadMap();
const char * getMapDllName();

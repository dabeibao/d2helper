#include <windows.h>
#include "MapHelper.hpp"
#include "loader.hpp"
#include "log.h"

#define NEW_MAP_NAME    "_d2hackmap.dll"

static HMODULE mapModule;
static const char * mapDllName;

static HMODULE loadFromFile()
{
    HMODULE hModule;

    hModule = tryLoadDll(NEW_MAP_NAME);
    if (hModule != nullptr) {
        mapDllName = NEW_MAP_NAME;
        mapModule = hModule;
        return hModule;
    }
    hModule = tryLoadDll(ORIG_MAP_NAME);
    if (hModule != nullptr) {
        mapDllName = ORIG_MAP_NAME;
        mapModule = hModule;
        return hModule;
    }
    return nullptr;
}

HMODULE loadMap()
{
    if (mapModule != nullptr) {
        return mapModule;
    }

    return loadFromFile();
}

const char * getMapDllName()
{
    return mapDllName;
}

#pragma once

#include <windows.h>
#include <string>
#include <format>
#include "cConfig.hpp"
#include "log.h"

#define SECTION_NAME    "registry"

class RegIni {
public:
    RegIni(const char * preName, const char * customName):
        mPreConfig(preName), mConfig(customName)
    {
    }

    LONG        save(const char * key, DWORD type, const BYTE * data, DWORD size)
    {
        auto section = mConfig.section(SECTION_NAME);

        if (type == REG_DWORD) {
            DWORD value = 0;
            if (data != NULL) {
                value = *(DWORD *)data;
            }
            auto s = dwordToIniString(value);
            section.save(key, s);
            log_trace("Save: %s -> %s\n", key, s.c_str());
            return ERROR_SUCCESS;
        }
        if (type == REG_SZ) {
            const char * value = "";
            if (data != NULL) {
                value = (const char *)data;
            }
            auto s = stringToIniString(value);
            section.save(key, s);
            log_trace("Save: %s -> %s\n", key, s.c_str());
            return ERROR_SUCCESS;
        }

        log_warn("Save: unsupport type %lu\n", type);

        return ERROR_FILE_INVALID;
    }

    std::string loadConfig(const char * key)
    {
        auto data = mConfig.loadString(SECTION_NAME, key);
        if (!data.empty()) {
            return data;
        }
        return mPreConfig.loadString(SECTION_NAME, key);
    }

    #define SET_VALUE(p, v)     if (p != NULL) *p = v

    LONG        load(const char * key, LPBYTE data, DWORD size, DWORD *outSize, LPDWORD type)
    {
        if (key == NULL) {
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }

        auto line = loadConfig(key);
        std::string out;
        DWORD loadType;
        bool ok = parseIniString(key, line, out, &loadType);
        if (!ok) {
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }
        // Only support string and int so far
        if (loadType != REG_SZ && loadType != REG_DWORD) {
            log_warn("Unsupport reg type %lu\n", loadType);
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }
        SET_VALUE(type, loadType);

        if (size == 0) {
            return ERROR_SUCCESS;
        }

        if (loadType == REG_DWORD) {
            *outSize = sizeof(DWORD);
            if (size < *outSize) {
                return ERROR_MORE_DATA;
            }
            if (data != NULL) {
                *(DWORD *)data = std::stoul(out);
                log_trace("load: %s -> %lu\n", key, *(DWORD *)data);
            }
            return ERROR_SUCCESS;
        }

        // string
        *outSize = out.size() + 1;
        if (size < *outSize) {
            return ERROR_MORE_DATA;
        }
        if (data != NULL) {
            memcpy(data, out.c_str(), out.size());
            data[out.size()] = '\0';
            log_trace("load: %s -> %s\n", key, data);
        }
        return ERROR_SUCCESS;
    }

private:
    static std::string  dwordToIniString(DWORD value)
    {
        char buf[32];

        snprintf(buf, sizeof(buf), "dword:%lu", value);
        return std::string(buf);
    }

    static std::string stringToIniString(const char * value)
    {
        return std::string(value);
    }

    bool parseIniString(const char * key, const std::string& value, std::string& out, DWORD * type)
    {
        if (value.empty()) {
            *type = REG_NONE;
            return false;
        }

        const char * dwordPrefix = "dword:";

        if (_strnicmp(dwordPrefix, value.c_str(), strlen(dwordPrefix)) == 0) {
            *type = REG_DWORD;
            out = value.substr(strlen(dwordPrefix));
            return true;
        }

        *type = REG_SZ;
        out = value;
        return true;
    }

private:
    Config      mPreConfig;
    Config      mConfig;
};

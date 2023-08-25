#pragma once

#include <windows.h>
#include <string>
#include <format>
#include "cConfig.hpp"
#include "log.h"

#define SECTION_NAME    "registry"

class RegIni {
public:
    class Guess {
    public:
        virtual DWORD   guess(const char *) = 0;
    };

    RegIni(const char * fileName, Guess *guesser = nullptr): mConfig(fileName), mGuesser(guesser)
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

    #define SET_VALUE(p, v)     if (p != NULL) *p = v

    LONG        load(const char * key, LPBYTE data, DWORD size, DWORD *outSize, LPDWORD type)
    {
        if (key == NULL) {
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }

        auto section = mConfig.section(SECTION_NAME);
        auto line = section.loadString(key);
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

        snprintf(buf, sizeof(buf), "4,%lu", value);
        return std::string(buf);
    }

    static std::string stringToIniString(const char * value)
    {
        char buf[1024];

        snprintf(buf, sizeof(buf), "1,%s", value);
        return std::string(buf);
    }

    bool parseIniString(const char * key, const std::string& value, std::string& out, DWORD * type)
    {
        if (value.empty()) {
            *type = REG_NONE;
            return false;
        }

        auto pos = value.find(',');
        if (pos == value.npos) {
            out = value;
            *type = guessType(key);
        } else {
            auto keyStr = value.substr(0, pos);
            *type = std::stoi(keyStr);
            out = value.substr(pos + 1);
        }
        if (*type == REG_NONE) {
            return false;
        }
        return true;
    }

    DWORD       guessType(const char * key)
    {
        if (mGuesser) {
            return mGuesser->guess(key);
        }
        return REG_NONE;
    }

private:
    Config      mConfig;
    Guess *     mGuesser;
};

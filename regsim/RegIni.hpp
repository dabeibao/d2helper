#pragma once

#include <windows.h>
#include <string>
#include <format>
#include <string_view>
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
            log_verbose("Save: %s -> %s\n", key, s.c_str());
            return ERROR_SUCCESS;
        }
        if (type == REG_SZ) {
            const char * value = "";
            if (data != NULL) {
                value = (const char *)data;
            }
            auto s = stringToIniString(value);
            section.save(key, s);
            log_verbose("Save: %s -> %s\n", key, s.c_str());
            return ERROR_SUCCESS;
        }

        if (type == REG_BINARY || type == REG_MULTI_SZ) {
            const char * prefix = type == REG_BINARY? hexPrefix() : mszPrefix();
            std::string s;
            if (data != nullptr) {
                s = prefix;
            } else {
                s = dataToIniHex((const char *)data, size, prefix);
            }
            section.save(key, s);
            log_verbose("Save: %s -> %s\n", key, s.c_str());
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
        std::vector<char> out;
        DWORD loadType;
        bool ok = parseIniString(key, line, out, &loadType);
        if (!ok) {
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }
        // Only support string and int so far
        if (loadType != REG_SZ && loadType != REG_DWORD &&
            loadType != REG_BINARY && loadType != REG_MULTI_SZ) {
            log_warn("Unsupport reg type %lu\n", loadType);
            SET_VALUE(type, 0);
            SET_VALUE(outSize, 0);
            return ERROR_FILE_NOT_FOUND;
        }
        SET_VALUE(type, loadType);

        if (loadType == REG_DWORD) {
            *outSize = sizeof(DWORD);
            if (size < *outSize) {
                return ERROR_MORE_DATA;
            }
            if (data != NULL) {
                std::string     s(out.begin(), out.end());
                *(DWORD *)data = std::stoul(s);
                log_verbose("load: %s -> %lu\n", key, *(DWORD *)data);
            }
            return ERROR_SUCCESS;
        }

        if (loadType == REG_BINARY || loadType == REG_MULTI_SZ) {
            *outSize = out.size();
            if (size < *outSize) {
                return ERROR_MORE_DATA;
            }
            if (data != nullptr) {
                memcpy(data, out.data(), out.size());
            }
            return ERROR_SUCCESS;
        }

        // string
        *outSize = out.size() + 1;
        if (size < *outSize) {
            return ERROR_MORE_DATA;
        }
        if (data != NULL) {
            memcpy(data, out.data(), out.size());
            data[out.size()] = '\0';
            log_verbose("load: %s -> %s\n", key, data);
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

    static std::string dataToHex(const char * value, unsigned int size)
    {
        std::string             hexString;
        static const char       map[] = "0123456789ABCDEF";

        hexString.reserve(2 * size + size - 1);
        for (unsigned int i = 0; i < size; i += 1) {
            unsigned char       c0 = value[i] & 0xf;
            unsigned char       c1 = (value[i] >> 4) & 0xf;
            hexString.push_back(map[c1]);
            hexString.push_back(map[c0]);
            if (i != size - 1) {
                hexString.push_back(',');
            }
        }
        return hexString;
    }

    static std::string dataToIniHex(const char * value, unsigned int size, const std::string_view& prefix)
    {
        auto    s = dataToHex(value, size);
        return std::string(prefix) + s;
    }

    static bool parseHex(std::string& value, std::vector<char>& out)
    {
        helper::remove(value, "\r\n\\");
        std::vector<std::string> list;
        helper::split(value, ',', list);
        for (auto &s: list) {
            bool ok;
            int v = helper::toInt(s, &ok, 16);
            if (!ok) {
                return false;
            }
            out.push_back(v & 0xff);
        }
        return true;
    }

    bool parseIniString(const char * key, std::string& value, std::vector<char>& out, DWORD * type)
    {
        if (value.empty()) {
            *type = REG_NONE;
            return false;
        }

        // REG_DWORD
        const char * dwordPrefix = "dword:";

        if (_strnicmp(dwordPrefix, value.c_str(), strlen(dwordPrefix)) == 0) {
            int         offset = strlen(dwordPrefix);
            *type = REG_DWORD;
            out.reserve(value.size() - offset);
            out.assign(value.begin() + offset, value.end());
            return true;
        }

        // REG_BINARY & REG_MULTI_SZ
        DWORD           hexTypes[] = {
            REG_BINARY, REG_MULTI_SZ,
        };
        const char *    hexPres[] = {
            hexPrefix(), mszPrefix(),
        };
        for (int i = 0; i < ARRAYSIZE(hexTypes); i += 1) {
            const char *        prefix = hexPres[i];
            int                 prefixLen = strlen(prefix);
            if (_strnicmp(prefix, value.c_str(), prefixLen) != 0) {
                continue;
            }
            *type = hexTypes[i];
            auto s = value.substr(prefixLen);
            bool ok = parseHex(s, out);
            return ok;
        }

        // For all other types

        *type = REG_SZ;
        out.reserve(value.size());
        out.assign(value.begin(), value.end());
        return true;
    }

    static const char *         hexPrefix()
    {
        return "hex:";
    }

    static const char *         mszPrefix()
    {
        return "hex(7):";
    }

private:
    Config      mPreConfig;
    Config      mConfig;
};

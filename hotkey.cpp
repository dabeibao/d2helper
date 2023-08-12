#include <windows.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include "hotkey.hpp"
#include "log.h"

static void split(const std::string& s, char delim, std::vector<std::string>& list)
{
    std::stringstream   ss(s);
    std::string         item;
    while (std::getline(ss, item, delim)) {
        list.push_back(std::move(item));
    }
}
// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

static bool getFunctionKey(const std::string& s, uint32_t * func)
{
    if (s.size() < 2 || s.size() > 3) {
        return false;
    }
    if (s[0] != 'f') {
        return false;
    }
    int index = 0;
    if (!isdigit(s[1])) {
        return false;
    }
    index = s[1] - '0';
    if (s.size() == 3) {
        if (!isdigit(s[2])) {
            return false;
        }
        index = index * 10 + (s[2] - '0');
    }
    if (index <= 0 || index > 24) {
        return false;
    }

    *func = VK_F1 + index - 1;
    return true;
}

static bool getCtrlKey(const std::string&s, uint32_t * func)
{
    static std::unordered_map<std::string, uint32_t> keys = {
        {"ctrl",        Hotkey::Ctrl},
        {"alt",         Hotkey::Alt},
        {"Shift",       Hotkey::Shift},

        {"tab",         VK_TAB},
        {"space",       VK_SPACE},
        {"backspace",   VK_BACK},
        {"home",        VK_HOME},
        {"end",         VK_END},
        {"left",        VK_LEFT},
        {"up",          VK_UP},
        {"right",       VK_RIGHT},
        {"down",        VK_DOWN},
        {"insert",      VK_INSERT},
        {"delete",      VK_DELETE},
        {"enter",       VK_RETURN},
        {"cap",         VK_CAPITAL},
        {"pgup",        VK_PRIOR},
        {"pgdn",        VK_DOWN},
        {"prtsc",       VK_PRINT},
        {"=",           VK_OEM_PLUS},
        {",",           VK_OEM_COMMA},
        {"-",           VK_OEM_MINUS},
        {".",           VK_OEM_PERIOD},
        {";",           VK_OEM_1},
        {"/",           VK_OEM_2},
        {"`",           VK_OEM_3},
        {"[",           VK_OEM_4},
        {"\\",          VK_OEM_5},
        {"]",           VK_OEM_6},
        {"'",           VK_OEM_7},
    };

    auto iter = keys.find(s);
    if (iter != keys.end()) {
        *func = iter->second;
        return true;
    }

    return false;
}

static uint32_t stringToKey(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    bool        ok;
    uint32_t    key = Hotkey::Invalid;

    ok = getCtrlKey(s, &key);
    if (ok) {
        return key;
    }

    ok = getFunctionKey(s, &key);
    if (ok) {
        return key;
    }

    if (s.starts_with("0x")) {
        return (uint32_t)std::stoul(s, nullptr, 0);
    }

    if (s.size() >= 2) {
        return Hotkey::Invalid;
    }

    unsigned char c = s[0];
    if (c >= '0' && c <= '9') {
        return (uint64_t)c;
    }

    if (c >= 'a' && c <= 'z') {
        return std::toupper(c);
    }

    return Hotkey::Invalid;
}

uint32_t Hotkey::parseKey(const std::string &s)
{
    if (s.empty()) {
        return Invalid;
    }

    uint32_t    combKey = Hotkey::Invalid;
    bool        hasNormal = false;

    std::vector<std::string>    list;
    split(s, '+', list);
    for (auto& e: list) {
        uint32_t       key = stringToKey(e);
        if (key == Hotkey::Invalid) {
            log_warn("Key %s is invalid\n", s.c_str());
            return Invalid;
        }
        if (Hotkey::isComb(key)) {
            combKey |= key;
            continue;
        }
        if (hasNormal) {
            log_warn("Key %s has more than one normal keys\n", s.c_str());
            return Invalid;
        }
        combKey |= key;
    }

    return combKey;
}

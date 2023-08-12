#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

class Hotkey {
public:
    static const uint32_t       Invalid = 0;
    static const uint32_t       Ctrl = 1u << 31;
    static const uint32_t       Alt = 1u << 30;
    static const uint32_t       Shift = 1ull << 29;

    static uint32_t     parseKey(const std::string& s);

    static bool         isComb(uint32_t key)
    {
        return (key & (Ctrl | Alt  | Shift)) != 0;
    }


    Hotkey(uint32_t value): mKey(value) { }
    Hotkey(const std::string& s)
    {
        mKey = parseKey(s);
    }

    uint32_t    key() const { return mKey; }

    bool        valid() const { return mKey != Invalid; }

    bool        operator==(const Hotkey& other) const
    {
        return mKey == other.mKey;
    }


private:
    uint32_t    mKey;
};

template <> struct std::hash<Hotkey> {
    std::size_t         operator() (const Hotkey& key) const
    {
        return key.key();
    }
};

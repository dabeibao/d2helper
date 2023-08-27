#pragma once

#include <vector>
#include <sstream>
#include <cctype>
#include <string>

namespace helper {
static inline void split(const std::string& s, char delim, std::vector<std::string>& list)
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

static inline int toInt(const std::string& s, bool * ok = nullptr, int base = 0)
{
    char *      errPtr;
    long        v = std::strtol(s.c_str(), &errPtr, base);

    if (ok != nullptr) {
        *ok = *errPtr == '\0';
    }

    return (int)v;
}

}

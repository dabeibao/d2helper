#pragma once

#include <windows.h>
#include <string>
#include <string.h>

class Config {
    static constexpr int        maxStringSize = 128;
public:
    class Section
    {
    public:
        Section(const Config& config, const char * sectionName):
            mConfig(config), mSection(sectionName)
        {
        }

        std::string loadString(const char * key, const std::string& def="") const
        {
            return mConfig.loadString(mSection, key, def);
        }

        double loadDouble(const char * key, double def = 0.0) const
        {
            return mConfig.loadDouble(mSection, key, def);
        }

        int loadInt(const char * key, int def = 0) const
        {
            return mConfig.loadInt(mSection, key, def);
        }

        bool loadBool(const char * key, bool def = false) const
        {
            return mConfig.loadBool(mSection, key, def);
        }

    private:
        const Config&   mConfig;
        const char *    mSection;
    };

    Config(const char * fileName = nullptr)
    {
        if (fileName != nullptr) {
            mFileName = fileName;
        } else {
            mFileName = "";
        }
    }

    Section section(const char * section) const
    {
        return Section(*this, section);
    }
    void setFileName(const char * fileName)
    {
        mFileName = fileName;
    }

    std::string loadString(const char * section, const char * name, const std::string& def = "") const
    {
        char    buf[maxStringSize];
        GetPrivateProfileStringA(section, name, def.c_str(), &buf[0], sizeof(buf), mFileName.c_str());
        return std::string(buf);
    }

    int loadInt(const char * section, const char * name, int def = 0) const
    {
        return (int)GetPrivateProfileIntA(section, name, def, mFileName.c_str());
    }

    double loadDouble(const char * section, const char * name, double def=0) const
    {
        auto str = loadString(section, name, "");
        try {
            return std::stod(str);
        } catch (...) {
            return def;
        }
    }

    bool loadBool(const char * section, const char * name, bool def = false) const
    {
        auto str = loadString(section, name, "");

        if (str.empty()) {
            return def;
        }

        if (_strcmpi(str.c_str(), "false") == 0) {
            return false;
        }
        if (_strcmpi(str.c_str(), "true") == 0) {
            return true;
        }

        try {
            int i = std::stoi(str);
            return i != 0;
        } catch (...) {
        }

        return def;
    }

private:
    std::string         mFileName;
};

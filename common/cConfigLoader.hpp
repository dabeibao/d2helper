#pragma once

#include "cConfig.hpp"
#include <string>
#include <direct.h>
#include "loader.hpp"

#define CONFIG_FILE_NAME       "d2ext.ini"

class CfgLoad
{
public:
    CfgLoad(const char * iniFile = CONFIG_FILE_NAME)
    {
        char path[512];
        bool ok = getAppDirectory(path, sizeof(path));
        if (!ok) {
            return;
        }
        std::string fileName(path);
        if (fileName.size() == 0 || fileName.c_str()[fileName.size() - 1] != '\\') {
            fileName += '\\';
        }
        fileName += iniFile;
        mConfig.setFileName(fileName.c_str());
    }

    static Config::Section      section(const char * sectionName)
    {
        return config().section(sectionName);
    }

    static const Config&        config()
    {
        return inst().mConfig;
    }

private:
    static const CfgLoad &      inst()
    {
        static CfgLoad gCfgLoad;
        return gCfgLoad;
    }

private:
    std::string         mFileName;
    Config              mConfig;
};

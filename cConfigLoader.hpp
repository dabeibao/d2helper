#pragma once

#include "cConfig.hpp"
#include <string>
#include <direct.h>

#define CONFIG_FILE_NAME       "d2ext.ini"

class CfgLoad
{
public:
    CfgLoad()
    {
        char * buf = _getcwd(NULL, 0);
        if (buf == nullptr) {
            return;
        }
        std::string fileName(buf);
        if (fileName.size() == 0 || fileName.c_str()[fileName.size() - 1] != '\\') {
            fileName += '\\';
        }
        fileName += CONFIG_FILE_NAME;
        mConfig.setFileName(fileName.c_str());
        free(buf);
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

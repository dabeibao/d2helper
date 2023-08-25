#include <windows.h>
#include <stdio.h>
#include "config.hpp"

#define APP_NAME        "config"
#define EN_STR          "enable"
#define VERBOSE_STR     "verbose"
#define DEBUG_STR       "debug"

static char configFileName[1024];

void config_load(const char * file, config_t *config)
{
    snprintf(configFileName, sizeof(configFileName), "%s", file);
    Config cfg(APP_NAME);
    config->is_enable = cfg.load(EN_STR, 1);
    config->is_verbose = cfg.load(VERBOSE_STR);
    config->is_debug = cfg.load(DEBUG_STR);
}

int Config::load(const char *key, int def)
{
    return GetPrivateProfileIntA(mKeyName, key, def, configFileName);
}

#include <windows.h>
#include <stdio.h>
#include "config.hpp"
#include "cConfigLoader.hpp"

#define APP_NAME        "config"
#define EN_STR          "enable"
#define VERBOSE_STR     "verbose"
#define DEBUG_STR       "debug"

void config_load(config_t *config)
{
    auto section = CfgLoad::section("helper.config");

    config->is_enable = section.loadInt("enable", 1);
    config->is_verbose = section.loadInt("verbose", 0);
    config->is_debug = section.loadInt("debug", 0);
    config->patch_delay = section.loadInt("delay", 0);
}


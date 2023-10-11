#pragma once
#include <stdint.h>
#include <stdio.h>

#define INI_FILE_NAME   "d2ext.ini"

struct config_t {
    int         is_verbose;
    int         is_enable;
    int         is_debug;
    int         patch_delay;
};

void config_load(const char * file, config_t *config);


class Config {
public:
    Config(const char * name)
    {
        snprintf(mKeyName, sizeof(mKeyName), "helper.%s", name);
    }
    int load(const char *key, int def = 0);

private:
    char                mKeyName[128];
};

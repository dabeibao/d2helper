#pragma once
#include <stdint.h>
#include <stdio.h>

struct config_t {
    int         is_verbose;
    int         is_enable;
    int         is_debug;
    int         patch_delay;
};

void config_load(config_t *config);

#include "macros.hpp"
#include "d2h_module.hpp"
#include "log.h"

extern D2HModule keyModule;
extern D2HModule fastCastModule;
extern D2HModule hackScriptModule;
extern D2HModule quickPotionModule;

static D2HModule * modules[] = {
    &keyModule,
    &fastCastModule,
    &quickPotionModule,
    &hackScriptModule,
};

static bool isModuleInited;

int module_init()
{
    int         i;

    log_trace("Module init entry");
    for (i = 0; i < ARRAY_SIZE(modules); i += 1) {
        log_trace("Init module %s\n", modules[i]->name);
        log_flush();

        int     ret = modules[i]->install();
        if (ret < 0) {
            log_warn("Init module %s failed %d\n", modules[i]->name, ret);
            goto failed;
        }
    }
    isModuleInited = true;

    return 0;


failed:
    for (int j = i - 1; j >= 0; j -= 1) {
        log_trace("Unload module %s\n", modules[j]->name);
        modules[j]->uninstall();
    }

    return -1;
}

void module_destroy()
{
    if (!isModuleInited) {
        return;
    }

    for (int i = ARRAY_SIZE(modules) - 1; i >= 0; i -= 1) {
        modules[i]->uninstall();
    }
    isModuleInited = false;
}

void module_on_load()
{
    if (!isModuleInited) {
        return;
    }
    for (auto m: modules) {
        if (m->onLoad != nullptr) {
            m->onLoad();
        }
    }
}
